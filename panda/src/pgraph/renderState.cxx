// Filename: renderState.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "renderState.h"
#include "billboardAttrib.h"
#include "transparencyAttrib.h"
#include "cullBinAttrib.h"
#include "cullBinManager.h"
#include "config_pgraph.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramIterator.h"
#include "indent.h"
#include "compareTo.h"

RenderState::States RenderState::_states;
CPT(RenderState) RenderState::_empty_state;
TypeHandle RenderState::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Constructor
//       Access: Protected
//  Description: Actually, this could be a private constructor, since
//               no one inherits from RenderState, but gcc gives us a
//               spurious warning if all constructors are private.
////////////////////////////////////////////////////////////////////
RenderState::
RenderState() {
  _saved_entry = _states.end();
  _self_compose = (RenderState *)NULL;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Copy Constructor
//       Access: Private
//  Description: RenderStates are not meant to be copied.
////////////////////////////////////////////////////////////////////
RenderState::
RenderState(const RenderState &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Copy Assignment Operator
//       Access: Private
//  Description: RenderStates are not meant to be copied.
////////////////////////////////////////////////////////////////////
void RenderState::
operator = (const RenderState &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Destructor
//       Access: Public, Virtual
//  Description: The destructor is responsible for removing the
//               RenderState from the global set if it is there.
////////////////////////////////////////////////////////////////////
RenderState::
~RenderState() {
  // Remove the deleted RenderState object from the global pool.
  if (_saved_entry != _states.end()) {
    _states.erase(_saved_entry);
    _saved_entry = _states.end();
  }

  // Now make sure we clean up all other floating pointers to the
  // RenderState.  These may be scattered around in the various
  // CompositionCaches from other RenderState objects.

  // Fortunately, since we added CompositionCache records in pairs, we
  // know exactly the set of RenderState objects that have us in their
  // cache: it's the same set of RenderState objects that we have in
  // our own cache.

  // We do need to put some thought into this loop, because as we
  // clear out cache entries we'll cause other RenderState objects to
  // destruct, which could cause things to get pulled out of our own
  // _composition_cache map.  We don't want to get bitten by this
  // cascading effect.
  CompositionCache::iterator ci;
  ci = _composition_cache.begin();
  while (ci != _composition_cache.end()) {
    {
      PT(RenderState) other = (RenderState *)(*ci).first;
      Composition comp = (*ci).second;

      // We should never have a reflexive entry in this map.  If we
      // do, something got screwed up elsewhere.
      nassertv(other != (const RenderState *)this);
      
      // Now we're holding a reference count to the other state, as well
      // as to the computed result (if any), so neither object will be
      // tempted to destruct.  Go ahead and remove ourselves from the
      // other cache.
      other->_composition_cache.erase(this);

      // It's all right if the other state destructs now, since it
      // won't try to remove itself from our own composition cache any
      // more.  Someone might conceivably delete the *next* entry,
      // though, so we should be sure to let all that deleting finish
      // up before we attempt to increment ci, by closing the scope
      // here.
    }
    // Now it's safe to increment ci, because the current cache entry
    // has not gone away, and if the next one has, by now it's safely
    // gone.
    ++ci;
  }

  // A similar bit of code for the invert cache.
  ci = _invert_composition_cache.begin();
  while (ci != _invert_composition_cache.end()) {
    {
      PT(RenderState) other = (RenderState *)(*ci).first;
      Composition comp = (*ci).second;
      nassertv(other != (const RenderState *)this);
      other->_invert_composition_cache.erase(this);
    }
    ++ci;
  }

  // Also, if we called compose(this) at some point and the return
  // value was something other than this, we need to decrement the
  // associated reference count.
  if (_self_compose != (RenderState *)NULL && _self_compose != this) {
    unref_delete((RenderState *)_self_compose);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::operator <
//       Access: Public
//  Description: Provides an arbitrary ordering among all unique
//               RenderStates, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               This method is not needed outside of the RenderState
//               class because all equivalent RenderState objects are
//               guaranteed to share the same pointer; thus, a pointer
//               comparison is always sufficient.
////////////////////////////////////////////////////////////////////
bool RenderState::
operator < (const RenderState &other) const {
  // We must compare all the properties of the attributes, not just
  // the type; thus, we compare them one at a time using compare_to().
  return lexicographical_compare(_attributes.begin(), _attributes.end(),
                                 other._attributes.begin(), other._attributes.end(),
                                 CompareTo<Attribute>());
}


////////////////////////////////////////////////////////////////////
//     Function: RenderState::find_attrib
//       Access: Published
//  Description: Searches for an attribute with the indicated type in
//               the state, and returns its index if it is found, or
//               -1 if it is not.
////////////////////////////////////////////////////////////////////
int RenderState::
find_attrib(TypeHandle type) const {
  Attributes::const_iterator ai = _attributes.find(Attribute(type));
  if (ai == _attributes.end()) {
    return -1;
  }
  return ai - _attributes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make_empty
//       Access: Published, Static
//  Description: Returns a RenderState with no attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make_empty() {
  // The empty state is asked for so often, we make it a special case
  // and store a pointer forever once we find it the first time.
  if (_empty_state == (RenderState *)NULL) {
    RenderState *state = new RenderState;
    _empty_state = return_new(state);
  }

  return _empty_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with one attribute set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib, int override) {
  RenderState *state = new RenderState;
  state->_attributes.reserve(1);
  state->_attributes.insert(Attribute(attrib, override));
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with two attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib1,
     const RenderAttrib *attrib2, int override) {
  RenderState *state = new RenderState;
  state->_attributes.reserve(2);
  state->_attributes.push_back(Attribute(attrib1, override));
  state->_attributes.push_back(Attribute(attrib2, override));
  state->_attributes.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with three attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib1,
     const RenderAttrib *attrib2,
     const RenderAttrib *attrib3, int override) {
  RenderState *state = new RenderState;
  state->_attributes.reserve(2);
  state->_attributes.push_back(Attribute(attrib1, override));
  state->_attributes.push_back(Attribute(attrib2, override));
  state->_attributes.push_back(Attribute(attrib3, override));
  state->_attributes.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with four attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib1,
     const RenderAttrib *attrib2,
     const RenderAttrib *attrib3,
     const RenderAttrib *attrib4, int override) {
  RenderState *state = new RenderState;
  state->_attributes.reserve(2);
  state->_attributes.push_back(Attribute(attrib1, override));
  state->_attributes.push_back(Attribute(attrib2, override));
  state->_attributes.push_back(Attribute(attrib3, override));
  state->_attributes.push_back(Attribute(attrib4, override));
  state->_attributes.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::compose
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               composition of this state with the other state.
//
//               The result of this operation is cached, and will be
//               retained as long as both this RenderState object and
//               the other RenderState object continue to exist.
//               Should one of them destruct, the cached entry will be
//               removed, and its pointer will be allowed to destruct
//               as well.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
compose(const RenderState *other) const {
  // This method isn't strictly const, because it updates the cache,
  // but we pretend that it is because it's only a cache which is
  // transparent to the rest of the interface.

  // We handle empty state (identity) as a trivial special case.
  if (is_empty()) {
    return other;
  }
  if (other->is_empty()) {
    return this;
  }

  if (other == this) {
    // compose(this) has to be handled as a special case, because the
    // caching problem is so different.
    if (_self_compose != (RenderState *)NULL) {
      return _self_compose;
    }
    CPT(RenderState) result = do_compose(this);
    ((RenderState *)this)->_self_compose = result;

    if (result != (const RenderState *)this) {
      // If the result of compose(this) is something other than this,
      // explicitly increment the reference count.  We have to be sure
      // to decrement it again later, in our destructor.
      _self_compose->ref();

      // (If the result was just this again, we still store the
      // result, but we don't increment the reference count, since
      // that would be a self-referential leak.  What a mess this is.)
    }
    return _self_compose;
  }

  // Is this composition already cached?
  CompositionCache::const_iterator ci = _composition_cache.find(other);
  if (ci != _composition_cache.end()) {
    const Composition &comp = (*ci).second;
    if (comp._result == (const RenderState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      ((Composition &)comp)._result = do_compose(other);
    }
    // Here's the cache!
    return comp._result;
  }

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other RenderState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(RenderState) result = do_compose(other);
  // We store them in this order, on the off-chance that other is the
  // same as this, a degenerate case which is still worth supporting.
  ((RenderState *)other)->_composition_cache[this]._result = NULL;
  ((RenderState *)this)->_composition_cache[other]._result = result;

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::invert_compose
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               composition of this state's inverse with the other
//               state.
//
//               This is similar to compose(), but is particularly
//               useful for computing the relative state of a node as
//               viewed from some other node.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
invert_compose(const RenderState *other) const {
  // This method isn't strictly const, because it updates the cache,
  // but we pretend that it is because it's only a cache which is
  // transparent to the rest of the interface.

  // We handle empty state (identity) as a trivial special case.
  if (is_empty()) {
    return other;
  }
  // Unlike compose(), the case of other->is_empty() is not quite as
  // trivial for invert_compose().

  if (other == this) {
    // a->invert_compose(a) always produces identity.
    return make_empty();
  }

  // Is this composition already cached?
  CompositionCache::const_iterator ci = _invert_composition_cache.find(other);
  if (ci != _invert_composition_cache.end()) {
    const Composition &comp = (*ci).second;
    if (comp._result == (const RenderState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      ((Composition &)comp)._result = do_invert_compose(other);
    }
    // Here's the cache!
    return comp._result;
  }

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other RenderState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(RenderState) result = do_invert_compose(other);
  // We store them in this order, on the off-chance that other is the
  // same as this, a degenerate case which is still worth supporting.
  ((RenderState *)other)->_invert_composition_cache[this]._result = NULL;
  ((RenderState *)this)->_invert_composition_cache[other]._result = result;

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::add_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the new RenderAttrib
//               added.  If there is already a RenderAttrib with the
//               same type, it is replaced.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
add_attrib(const RenderAttrib *attrib, int override) const {
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  Attribute new_attribute(attrib, override);
  Attributes::const_iterator ai = _attributes.begin();

  while (ai != _attributes.end() && (*ai) < new_attribute) {
    *result = *ai;
    ++ai;
    ++result;
  }
  *result = new_attribute;
  ++result;

  if (ai != _attributes.end() && !(new_attribute < (*ai))) {
    // At this point we know:
    // !((*ai) < new_attribute) && !(new_attribute < (*ai))
    // which means (*ai) == new_attribute--so we should leave it out,
    // to avoid duplicating attributes in the set.
    ++ai;
  }

  while (ai != _attributes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::remove_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the indicated
//               RenderAttrib removed
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
remove_attrib(TypeHandle type) const {
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  Attributes::const_iterator ai = _attributes.begin();

  while (ai != _attributes.end()) {
    if ((*ai)._type != type) {
      *result = *ai;
      ++result;
    }
    ++ai;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib
//       Access: Published, Virtual
//  Description: Looks for a RenderAttrib of the indicated type in the
//               state, and returns it if it is found, or NULL if it
//               is not.
////////////////////////////////////////////////////////////////////
const RenderAttrib *RenderState::
get_attrib(TypeHandle type) const {
  Attributes::const_iterator ai;
  ai = _attributes.find(Attribute(type));
  if (ai != _attributes.end()) {
    return (*ai)._attrib;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderState::
output(ostream &out) const {
  out << "S:";
  if (_attributes.empty()) {
    out << "(empty)";

  } else {
    Attributes::const_iterator ai = _attributes.begin();
    out << "(" << (*ai)._type;
    ++ai;
    while (ai != _attributes.end()) {
      out << " " << (*ai)._type;
      ++ai;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderState::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _attributes.size() << " attribs:\n";
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    const Attribute &attribute = (*ai);
    attribute._attrib->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::issue_delta_modify
//       Access: Public
//  Description: This is intended to be called only from
//               GraphicsStateGuardian::modify_state().  It calls
//               issue() for each attribute given in the other state
//               that differs from the current state (which is assumed
//               to represent the GSG's current state).  Returns the
//               RenderState representing the newly composed result.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
issue_delta_modify(const RenderState *other, 
                   GraphicsStateGuardianBase *gsg) const {
  if (other == this) {
    // If the state doesn't change, that's a trivial special case.
    return other;
  }

  // First, build a new Attributes member that represents the union of
  // this one and that one.
  Attributes::const_iterator ai = _attributes.begin();
  Attributes::const_iterator bi = other->_attributes.begin();

  // Create a new RenderState that will hold the result.
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  bool any_changed = false;

  while (ai != _attributes.end() && bi != other->_attributes.end()) {
    if ((*ai) < (*bi)) {
      // Here is an attribute that we have in the original, which is
      // not present in the secondary.  Leave it alone.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new attribute we have in the secondary, that was
      // not present in the original.  Issue the new one, and save it.
      (*bi)._attrib->issue(gsg);
      *result = *bi;
      ++bi;
      ++result;
      any_changed = true;
    } else {
      // Here is an attribute we have in both.  Issue the new one if
      // it's different, and save it.
      if ((*ai)._attrib != (*bi)._attrib) {
        any_changed = true;
        (*bi)._attrib->issue(gsg);
      }
      *result = *bi;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _attributes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != other->_attributes.end()) {
    (*bi)._attrib->issue(gsg);
    *result = *bi;
    ++bi;
    ++result;
    any_changed = true;
  }

  if (any_changed) {
    return return_new(new_state);
  } else {
    delete new_state;
    return other;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::issue_delta_set
//       Access: Public
//  Description: This is intended to be called only from
//               GraphicsStateGuardian::set_state().  It calls issue()
//               for each attribute given in the other state that
//               differs from the current state (which is assumed to
//               represent the GSG's current state).  Returns the
//               RenderState representing the newly composed result
//               (which will be the same as other).
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
issue_delta_set(const RenderState *other, 
                GraphicsStateGuardianBase *gsg) const {
  if (other->is_empty()) {
    // If the other state is empty, that's a trivial special case.
    return this;
  }

  // We don't need to build a new RenderState, because the return
  // value will be exactly other.

  Attributes::const_iterator ai = _attributes.begin();
  Attributes::const_iterator bi = other->_attributes.begin();

  while (ai != _attributes.end() && bi != other->_attributes.end()) {
    if ((*ai) < (*bi)) {
      // Here is an attribute that we have in the original, which is
      // not present in the secondary.  Issue the default state instead.
      (*ai)._attrib->make_default()->issue(gsg);
      ++ai;

    } else if ((*bi) < (*ai)) {
      // Here is a new attribute we have in the secondary, that was
      // not present in the original.  Issue the new one.
      (*bi)._attrib->issue(gsg);
      ++bi;

    } else {
      // Here is an attribute we have in both.  Issue the new one if
      // it's different.
      if ((*ai)._attrib != (*bi)._attrib) {
        (*bi)._attrib->issue(gsg);
      }
      ++ai;
      ++bi;
    }
  }

  while (ai != _attributes.end()) {
    (*ai)._attrib->make_default()->issue(gsg);
    ++ai;
  }

  while (bi != other->_attributes.end()) {
    (*bi)._attrib->issue(gsg);
    ++bi;
  }

  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::bin_removed
//       Access: Public, Static
//  Description: Intended to be called by
//               CullBinManager::remove_bin(), this informs all the
//               RenderStates in the world to remove the indicated
//               bin_index from their cache if it has been cached.
////////////////////////////////////////////////////////////////////
void RenderState::
bin_removed(int bin_index) {
  // Do something here.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::return_new
//       Access: Private, Static
//  Description: This function is used to share a common RenderState
//               pointer for all equivalent RenderState objects.
//
//               See the similar logic in RenderAttrib.  The idea is
//               to create a new RenderState object and pass it
//               through this function, which will share the pointer
//               with a previously-created RenderState object if it is
//               equivalent.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
return_new(RenderState *state) {
  nassertr(state != (RenderState *)NULL, state);

  // This should be a newly allocated pointer, not one that was used
  // for anything else.
  nassertr(state->_saved_entry == _states.end(), state);

  // Save the state in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(RenderState) pt_state = state;

  pair<States::iterator, bool> result = _states.insert(state);
  if (result.second) {
    // The state was inserted; save the iterator and return the
    // input state.
    state->_saved_entry = result.first;
    return pt_state;
  }

  // The state was not inserted; there must be an equivalent one
  // already in the set.  Return that one.
  return *(result.first);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_compose
//       Access: Private
//  Description: The private implemention of compose(); this actually
//               composes two RenderStates, without bothering with the
//               cache.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
do_compose(const RenderState *other) const {
  // First, build a new Attributes member that represents the union of
  // this one and that one.
  Attributes::const_iterator ai = _attributes.begin();
  Attributes::const_iterator bi = other->_attributes.begin();

  // Create a new RenderState that will hold the result.
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  while (ai != _attributes.end() && bi != other->_attributes.end()) {
    if ((*ai) < (*bi)) {
      // Here is an attribute that we have in the original, which is
      // not present in the secondary.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new attribute we have in the secondary, that was
      // not present in the original.
      *result = *bi;
      ++bi;
      ++result;
    } else {
      // Here is an attribute we have in both.  Does one override the
      // other?
      const Attribute &a = (*ai);
      const Attribute &b = (*bi);
      if (a._override < b._override) {
        // B overrides.
        *result = *bi;

      } else if (b._override < a._override) {
        // A overrides.
        *result = *ai;

      } else {
        // No, they're equivalent, so compose them.
        *result = Attribute(a._attrib->compose(b._attrib), b._override);
      }
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _attributes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != other->_attributes.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_invert_compose
//       Access: Private
//  Description: The private implemention of invert_compose().
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
do_invert_compose(const RenderState *other) const {
  Attributes::const_iterator ai = _attributes.begin();
  Attributes::const_iterator bi = other->_attributes.begin();

  // Create a new RenderState that will hold the result.
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  while (ai != _attributes.end() && bi != other->_attributes.end()) {
    if ((*ai) < (*bi)) {
      // Here is an attribute that we have in the original, which is
      // not present in the secondary.
      *result = Attribute((*ai)._attrib->invert_compose((*ai)._attrib->make_default()), 0);
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new attribute we have in the secondary, that was
      // not present in the original.
      *result = *bi;
      ++bi;
      ++result;
    } else {
      // Here is an attribute we have in both.  In this case, override
      // is meaningless.
      *result = Attribute((*ai)._attrib->invert_compose((*bi)._attrib), (*bi)._override);
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _attributes.end()) {
    *result = Attribute((*ai)._attrib->invert_compose((*ai)._attrib->make_default()), 0);
    ++ai;
    ++result;
  }

  while (bi != other->_attributes.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_billboard
//       Access: Private
//  Description: This is the private implementation of
//               get_billboard().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_billboard() {
  const RenderAttrib *attrib = get_attrib(BillboardAttrib::get_class_type());
  _billboard = (const BillboardAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _billboard = DCAST(BillboardAttrib, attrib);
  }
  _flags |= F_checked_billboard;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_bin_index
//       Access: Private
//  Description: This is the private implementation of
//               get_bin_index() and get_draw_order().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_bin_index() {
  string bin_name;
  _draw_order = 0;

  const RenderAttrib *attrib = get_attrib(CullBinAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const CullBinAttrib *bin_attrib = DCAST(CullBinAttrib, attrib);
    bin_name = bin_attrib->get_bin_name();
    _draw_order = bin_attrib->get_draw_order();
  }

  if (bin_name.empty()) {
    // No explicit bin is specified; put in the in the default bin,
    // either opaque or transparent, based on the transparency
    // setting.
    bin_name = "opaque";
    const RenderAttrib *attrib = get_attrib(TransparencyAttrib::get_class_type());
    if (attrib != (const RenderAttrib *)NULL) {
      const TransparencyAttrib *trans = DCAST(TransparencyAttrib, attrib);
      switch (trans->get_mode()) {
      case TransparencyAttrib::M_alpha:
      case TransparencyAttrib::M_alpha_sorted:
      case TransparencyAttrib::M_binary:
        // These transparency modes require special back-to-front sorting.
        bin_name = "transparent";
        break;

      default:
        break;
      }
    }
  }

  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  _bin_index = bin_manager->find_bin(bin_name);
  if (_bin_index == -1) {
    pgraph_cat.warning()
      << "No bin named " << bin_name << "; creating default bin.\n";
    _bin_index = bin_manager->add_bin(bin_name, CullBinManager::BT_unsorted, 0);
  }
  _flags |= F_checked_bin_index;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               RenderState.
////////////////////////////////////////////////////////////////////
void RenderState::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RenderState::
write_datagram(BamWriter *manager, Datagram &dg) {
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::finalize
//       Access: Public, Virtual
//  Description: Method to ensure that any necessary clean up tasks
//               that have to be performed by this object are performed
////////////////////////////////////////////////////////////////////
void RenderState::
finalize() {
  // Unref the pointer that we explicitly reffed in make_from_bam().
  unref();

  // We should never get back to zero after unreffing our own count,
  // because we expect to have been stored in a pointer somewhere.  If
  // we do get to zero, it's a memory leak; the way to avoid this is
  // to call unref_delete() above instead of unref(), but this is
  // dangerous to do from within a virtual function.
  nassertv(get_ref_count() != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type RenderState is encountered
//               in the Bam file.  It should create the RenderState
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderState::
make_from_bam(const FactoryParams &params) {
  RenderState *state = new RenderState;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  state->fillin(scan, manager);

  return new_from_bam(state, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::new_from_bam
//       Access: Protected, Static
//  Description: Uniquifies the pointer for a RenderState object just
//               created from a bam file, and preserves its reference
//               count correctly.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderState::
new_from_bam(RenderState *state, BamReader *manager) {
  // First, uniquify the pointer.
  CPT(RenderState) pointer = return_new(state);

  // But now we have a problem, since we have to hold the reference
  // count and there's no way to return a TypedWritable while still
  // holding the reference count!  We work around this by explicitly
  // upping the count, and also setting a finalize() callback to down
  // it later.
  if (pointer == state) {
    pointer->ref();
    manager->register_finalize(state);
  }
  
  // We have to cast the pointer back to non-const, because the bam
  // reader expects that.
  return (RenderState *)pointer.p();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RenderState.
////////////////////////////////////////////////////////////////////
void RenderState::
fillin(DatagramIterator &scan, BamReader *manager) {
}
