// Filename: eggJointData.cxx
// Created by:  drose (23Feb01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "eggJointData.h"
#include "eggJointNodePointer.h"
#include "eggMatrixTablePointer.h"

#include "dcast.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "indent.h"
#include "fftCompressor.h"
#include "zStream.h"

TypeHandle EggJointData::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggJointData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggJointData::
EggJointData(EggCharacterCollection *collection,
             EggCharacterData *char_data) :
  EggComponentData(collection, char_data)
{
  _parent = (EggJointData *)NULL;
  _new_parent = (EggJointData *)NULL;
  _has_rest_frame = false;
  _rest_frames_differ = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_frame
//       Access: Public
//  Description: Returns the local transform matrix corresponding to
//               this joint position in the nth frame in the indicated
//               model.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_frame(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  return joint->get_frame(n);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_net_frame
//       Access: Public
//  Description: Returns the complete transform from the root
//               corresponding to this joint position in the nth frame
//               in the indicated model.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_net_frame(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  if (joint->get_num_net_frames() < n) {
    // Recursively get the previous frame's net, so we have a place to
    // stuff this frame's value.
    get_net_frame(model_index, n - 1);
  }

  if (joint->get_num_net_frames() == n) {
    // Compute this frame's net, and stuff it in.
    LMatrix4d mat = get_frame(model_index, n);
    if (_parent != (EggJointData *)NULL) {
      mat = mat * _parent->get_net_frame(model_index, n);
    }
    joint->add_net_frame(mat);
  }

  return joint->get_net_frame(n);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_net_frame_inv
//       Access: Public
//  Description: Returns the inverse of get_net_frame().
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_net_frame_inv(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  if (joint->get_num_net_frame_invs() < n) {
    // Recursively get the previous frame's net, so we have a place to
    // stuff this frame's value.
    get_net_frame_inv(model_index, n - 1);
  }

  if (joint->get_num_net_frame_invs() == n) {
    // Compute this frame's net inverse, and stuff it in.
    LMatrix4d mat = get_net_frame(model_index, n);
    mat.invert_in_place();
    joint->add_net_frame_inv(mat);
  }

  return joint->get_net_frame_inv(n);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::force_initial_rest_frame
//       Access: Public
//  Description: Forces all of the joints to have the same rest frame
//               value as the first joint read in.  This is a drastic
//               way to repair models whose rest frame values are
//               completely bogus, but should not be performed on
//               models that are otherwise correct.
////////////////////////////////////////////////////////////////////
void EggJointData::
force_initial_rest_frame() {
  if (!has_rest_frame()) {
    return;
  }
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, get_model(model_index));
      if (joint->is_of_type(EggJointNodePointer::get_class_type())) {
        joint->set_frame(0, get_rest_frame());
      }
    }
  }
  _rest_frames_differ = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::move_vertices_to
//       Access: Public
//  Description: Moves the vertices assigned to this joint into the
//               indicated joint, without changing their weight
//               assignments.
////////////////////////////////////////////////////////////////////
void EggJointData::
move_vertices_to(EggJointData *new_owner) {
  int num_models = get_num_models();

  if (new_owner == (EggJointData *)NULL) {
    for (int model_index = 0; model_index < num_models; model_index++) {
      if (has_model(model_index)) {
        EggJointPointer *joint;
        DCAST_INTO_V(joint, get_model(model_index));
        joint->move_vertices_to((EggJointPointer *)NULL);
      }
    }
  } else {
    for (int model_index = 0; model_index < num_models; model_index++) {
      if (has_model(model_index) && new_owner->has_model(model_index)) {
        EggJointPointer *joint, *new_joint;
        DCAST_INTO_V(joint, get_model(model_index));
        DCAST_INTO_V(new_joint, new_owner->get_model(model_index));
        joint->move_vertices_to(new_joint);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::score_reparent_to
//       Access: Public
//  Description: Computes a score >= 0 reflecting the similarity of
//               the current joint's animation (in world space) to
//               that of the indicated potential parent joint (in
//               world space).  The lower the number, the more similar
//               the motion, and the more suitable is the proposed
//               parent-child relationship.  Returns -1 if there is an
//               error.
////////////////////////////////////////////////////////////////////
int EggJointData::
score_reparent_to(EggJointData *new_parent) {
  if (!FFTCompressor::is_compression_available()) {
    // If we don't have compression compiled in, we can't meaningfully
    // score the joints.
    return -1;
  }

  // First, build up a big array of the new transforms this joint
  // would receive in all frames of all models, were it reparented to
  // the indicated joint.
  vector_float i, j, k, a, b, c, x, y, z;
  vector_LVecBase3f hprs;
  int num_rows = 0;

  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    EggBackPointer *back = get_model(model_index);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, back, false);

      int num_frames = get_num_frames(model_index);
      for (int n = 0; n < num_frames; n++) {
        LMatrix4d transform;
        if (_parent == new_parent) {
          // We already have this parent.
          transform = LMatrix4d::ident_mat();
          
        } else if (_parent == (EggJointData *)NULL) {
          // We are moving from outside the joint hierarchy to within it.
          transform = new_parent->get_net_frame_inv(model_index, n);
          
        } else if (new_parent == (EggJointData *)NULL) {
          // We are moving from within the hierarchy to outside it.
          transform = _parent->get_net_frame(model_index, n);
          
        } else {
          // We are changing parents within the hierarchy.
          transform = 
            _parent->get_net_frame(model_index, n) *
            new_parent->get_net_frame_inv(model_index, n);
        }

        transform = joint->get_frame(n) * transform;
        LVecBase3d scale, shear, hpr, translate;
        if (!decompose_matrix(transform, scale, shear, hpr, translate)) {
          // Invalid transform.
          return -1;
        }
        i.push_back(scale[0]);
        j.push_back(scale[1]);
        k.push_back(scale[2]);
        a.push_back(shear[0]);
        b.push_back(shear[1]);
        c.push_back(shear[2]);
        hprs.push_back(LCAST(float, hpr));
        x.push_back(translate[0]);
        y.push_back(translate[1]);
        z.push_back(translate[2]);
        num_rows++;
      }
    }
  }

  if (num_rows == 0) {
    // No data, no score.
    return -1;
  }

  // Now, we derive a score, by the simple expedient of using the
  // FFTCompressor to compress the generated transforms, and measuring
  // the length of the resulting bitstream.
  FFTCompressor compressor;
  Datagram dg;
  compressor.write_reals(dg, &i[0], num_rows);
  compressor.write_reals(dg, &j[0], num_rows);
  compressor.write_reals(dg, &k[0], num_rows);
  compressor.write_reals(dg, &a[0], num_rows);
  compressor.write_reals(dg, &b[0], num_rows);
  compressor.write_reals(dg, &c[0], num_rows);
  compressor.write_hprs(dg, &hprs[0], num_rows);
  compressor.write_reals(dg, &x[0], num_rows);
  compressor.write_reals(dg, &y[0], num_rows);
  compressor.write_reals(dg, &z[0], num_rows);


#ifndef HAVE_ZLIB
  return dg.get_length();

#else
  // The FFTCompressor does minimal run-length encoding, but to really
  // get an accurate measure we should zlib-compress the resulting
  // stream.
  ostringstream sstr;
  OCompressStream zstr(&sstr, false);
  zstr.write((const char *)dg.get_data(), dg.get_length());
  zstr.flush();
  return sstr.str().length();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_rebuild
//       Access: Public
//  Description: Calls do_rebuild() on all models, and recursively on
//               all joints at this node and below.  Returns true if
//               all models returned true, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggJointData::
do_rebuild() {
  bool all_ok = true;

  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, back, false);
      if (!joint->do_rebuild()) {
        all_ok = false;
      }
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (!child->do_rebuild()) {
      all_ok = false;
    }
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::optimize
//       Access: Public
//  Description: Calls optimize() on all models, and recursively on
//               all joints at this node and below.
////////////////////////////////////////////////////////////////////
void EggJointData::
optimize() {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->optimize();
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    child->optimize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::expose
//       Access: Public
//  Description: Calls expose() on all models for this joint, but does
//               not recurse downwards.
////////////////////////////////////////////////////////////////////
void EggJointData::
expose(EggGroup::DCSType dcs_type) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->expose(dcs_type);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::zero_channels
//       Access: Public
//  Description: Calls zero_channels() on all models for this joint,
//               but does not recurse downwards.
////////////////////////////////////////////////////////////////////
void EggJointData::
zero_channels(const string &components) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->zero_channels(components);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::quantize_channels
//       Access: Public
//  Description: Calls quantize_channels() on all models for this joint,
//               and then recurses downwards to all joints below.
////////////////////////////////////////////////////////////////////
void EggJointData::
quantize_channels(const string &components, double quantum) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->quantize_channels(components, quantum);
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    child->quantize_channels(components, quantum);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::add_back_pointer
//       Access: Public, Virtual
//  Description: Adds the indicated model joint or anim table to the
//               data.
////////////////////////////////////////////////////////////////////
void EggJointData::
add_back_pointer(int model_index, EggObject *egg_object) {
  nassertv(egg_object != (EggObject *)NULL);
  if (egg_object->is_of_type(EggGroup::get_class_type())) {
    // It must be a <Joint>.
    EggJointNodePointer *joint = new EggJointNodePointer(egg_object);
    set_model(model_index, joint);
    if (!_has_rest_frame) {
      _rest_frame = joint->get_frame(0);
      _has_rest_frame = true;

    } else {
      // If this new node doesn't come within an acceptable tolerance
      // of our first reading of this joint's rest frame, set a
      // warning flag.
      if (!_rest_frame.almost_equal(joint->get_frame(0), 0.0001)) {
        _rest_frames_differ = true;
      }
    }

  } else if (egg_object->is_of_type(EggTable::get_class_type())) {
    // It's a <Table> with an "xform" child beneath it.
    EggMatrixTablePointer *xform = new EggMatrixTablePointer(egg_object);
    set_model(model_index, xform);

  } else {
    nout << "Invalid object added to joint for back pointer.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void EggJointData::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Joint " << get_name()
    << " (models:";
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
      out << " " << model_index;
    }
  }
  out << ") {\n";

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_begin_reparent
//       Access: Protected
//  Description: Clears out the _children vector in preparation for
//               refilling it from the _new_parent information.
////////////////////////////////////////////////////////////////////
void EggJointData::
do_begin_reparent() {
  _got_new_parent_depth = false;
  _children.clear();

  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, get_model(model_index));
      joint->begin_rebuild();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::calc_new_parent_depth
//       Access: Protected
//  Description: Calculates the number of joints above this joint in its
//               intended position, as specified by a recent call to
//               reparent_to(), and also checks for a cycle in the new
//               parent chain.  Returns true if a cycle is detected,
//               and false otherwise.  If a cycle is not detected,
//               _new_parent_depth can be consulted for the depth in
//               the new hierarchy.
//
//               This is used by EggCharacterData::do_reparent() to
//               determine the order in which to apply the reparent
//               operations.  It should be called after
//               do_begin_reparent().
////////////////////////////////////////////////////////////////////
bool EggJointData::
calc_new_parent_depth(pset<EggJointData *> &chain) {
  if (_got_new_parent_depth) {
    return false;
  }
  if (_new_parent == (EggJointData *)NULL) {
    // Here's the top of the new hierarchy.
    _got_new_parent_depth = true;
    _new_parent_depth = 0;
    return false;
  }
  if (!chain.insert(this).second) {
    // We've already visited this joint; that means there's a cycle.
    return true;
  }
  bool cycle = _new_parent->calc_new_parent_depth(chain);
  _new_parent_depth = _new_parent->_new_parent_depth + 1;
  return cycle;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_begin_compute_reparent
//       Access: Protected
//  Description: Eliminates any cached values before beginning a walk
//               through all the joints for do_compute_reparent(), for
//               a given model/frame.
////////////////////////////////////////////////////////////////////
void EggJointData::
do_begin_compute_reparent() { 
  _got_new_net_frame = false;
  _got_new_net_frame_inv = false;
  _computed_reparent = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_compute_reparent
//       Access: Protected
//  Description: Prepares the reparent operation by computing a new
//               transform for each frame of each model, designed to
//               keep the net transform the same when the joint is
//               moved to its new parent.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool EggJointData::
do_compute_reparent(int model_index, int n) {
  if (_computed_reparent) {
    // We've already done this joint.  This is possible because we
    // have to recursively compute joints upwards, so we might visit
    // the same joint more than once.
    return _computed_ok;
  }
  _computed_reparent = true;

  if (_parent == _new_parent) {
    // Trivial (and most common) case: we are not moving the joint.
    // No recomputation necessary.
    _computed_ok = true;
    return true;
  }

  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    // This joint doesn't have any data to modify.
    _computed_ok = true;
    return true;
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, false);

  LMatrix4d transform;
  if (_parent == (EggJointData *)NULL) {
    // We are moving from outside the joint hierarchy to within it.
    transform = _new_parent->get_new_net_frame_inv(model_index, n);

  } else if (_new_parent == (EggJointData *)NULL) {
    // We are moving from within the hierarchy to outside it.
    transform = _parent->get_net_frame(model_index, n);

  } else {
    // We are changing parents within the hierarchy.
    transform = 
      _parent->get_net_frame(model_index, n) *
      _new_parent->get_new_net_frame_inv(model_index, n);
  }

  nassertr(n == joint->get_num_rebuild_frames(), false);

  _computed_ok = joint->add_rebuild_frame(joint->get_frame(n) * transform);
  return _computed_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_finish_reparent
//       Access: Protected
//  Description: Performs the actual reparenting operation
//               by removing all of the old children and replacing
//               them with the set of new children.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool EggJointData::
do_finish_reparent() {
  bool all_ok = true;

  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    EggJointPointer *parent_joint = NULL;
    if (_new_parent != NULL && _new_parent->has_model(model_index)) {
      DCAST_INTO_R(parent_joint, _new_parent->get_model(model_index), false);
    }

    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, get_model(model_index), false);
      joint->do_finish_reparent(parent_joint);
      joint->clear_net_frames();
      if (!joint->do_rebuild()) {
        all_ok = false;
      }
    }
  }

  _parent = _new_parent;
  if (_parent != (EggJointData *)NULL) {
    _parent->_children.push_back(this);
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::make_new_joint
//       Access: Private
//  Description: Creates a new joint as a child of this joint and
//               returns it.  This is intended to be called only from
//               EggCharacterData::make_new_joint().
////////////////////////////////////////////////////////////////////
EggJointData *EggJointData::
make_new_joint(const string &name) {
  EggJointData *child = new EggJointData(_collection, _char_data);
  child->set_name(name);
  child->_parent = this;
  child->_new_parent = this;
  _children.push_back(child);

  // Also create new back pointers in each of the models.
  int num_models = get_num_models();
  for (int i = 0; i < num_models; i++) {
    if (has_model(i)) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, get_model(i), NULL);
      EggJointPointer *new_joint = joint->make_new_joint(name);
      child->set_model(i, new_joint);
    }
  }

  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::find_joint_exact
//       Access: Private
//  Description: The recursive implementation of find_joint, this
//               flavor searches recursively for an exact match of the
//               preferred joint name.
////////////////////////////////////////////////////////////////////
EggJointData *EggJointData::
find_joint_exact(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    EggJointData *result = child->find_joint_exact(name);
    if (result != (EggJointData *)NULL) {
      return result;
    }
  }

  return (EggJointData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::find_joint_matches
//       Access: Private
//  Description: The recursive implementation of find_joint, this
//               flavor searches recursively for any acceptable match.
////////////////////////////////////////////////////////////////////
EggJointData *EggJointData::
find_joint_matches(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->matches_name(name)) {
      return child;
    }
    EggJointData *result = child->find_joint_matches(name);
    if (result != (EggJointData *)NULL) {
      return result;
    }
  }

  return (EggJointData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::is_new_ancestor
//       Access: Protected
//  Description: Returns true if this joint is an ancestor of the
//               indicated joint, in the "new" hierarchy (that is, the
//               one defined by _new_parent, as set by reparent_to()
//               before do_finish_reparent() is called).
////////////////////////////////////////////////////////////////////
bool EggJointData::
is_new_ancestor(EggJointData *child) const {
  if (child == this) {
    return true;
  }

  if (child->_new_parent == (EggJointData *)NULL) {
    return false;
  }

  return is_new_ancestor(child->_new_parent);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_new_net_frame
//       Access: Private
//  Description: Similar to get_net_frame(), but computed for the
//               prospective new parentage of the node, before
//               do_finish_reparent() is called.  This is generally
//               useful only when called within do_compute_reparent().
////////////////////////////////////////////////////////////////////
const LMatrix4d &EggJointData::
get_new_net_frame(int model_index, int n) {
  if (!_got_new_net_frame) {
    _new_net_frame = get_new_frame(model_index, n);
    if (_new_parent != (EggJointData *)NULL) {
      _new_net_frame = _new_net_frame * _new_parent->get_new_net_frame(model_index, n);
    }
    _got_new_net_frame = true;
  }
  return _new_net_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_new_net_frame_inv
//       Access: Private
//  Description: Returns the inverse of get_new_net_frame().
////////////////////////////////////////////////////////////////////
const LMatrix4d &EggJointData::
get_new_net_frame_inv(int model_index, int n) {
  if (!_got_new_net_frame_inv) {
    _new_net_frame_inv.invert_from(get_new_frame(model_index, n));
    if (_new_parent != (EggJointData *)NULL) {
      _new_net_frame_inv = _new_parent->get_new_net_frame_inv(model_index, n) * _new_net_frame_inv;
    }
    _got_new_net_frame_inv = true;
  }
  return _new_net_frame_inv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_new_frame
//       Access: Private
//  Description: Returns the local transform matrix corresponding to
//               this joint position in the nth frame in the indicated
//               model, as it will be when do_finish_reparent() is
//               called.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_new_frame(int model_index, int n) {
  do_compute_reparent(model_index, n);

  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  if (joint->get_num_rebuild_frames() > 0) {
    return joint->get_rebuild_frame(n);
  } else {
    return joint->get_frame(n);
  }
}
