// Filename: depthTestTransition.cxx
// Created by:  mike (28Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "depthTestTransition.h"
#include "depthTestAttribute.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#include <indent.h>

TypeHandle DepthTestTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthTestTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DepthTestTransition::
make_copy() const {
  return new DepthTestTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthTestAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthTestTransition::
make_attrib() const {
  return new DepthTestAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another DepthTestTransition.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
set_value_from(const OnTransition *other) {
  const DepthTestTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int DepthTestTransition::
compare_values(const OnTransition *other) const {
  const DepthTestTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a DepthTestTransition object
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_DepthTestTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnTransition::write_datagram(manager, me);
  _value.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::make_DepthTestTransition
//       Access: Protected
//  Description: Factory method to generate a DepthTestTransition object
////////////////////////////////////////////////////////////////////
TypedWriteable* DepthTestTransition::
make_DepthTestTransition(const FactoryParams &params) {
  DepthTestTransition *me = new DepthTestTransition;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
fillin(DatagramIterator& scan, BamReader* manager) {
  OnTransition::fillin(scan, manager);
  _value.read_datagram(scan);
}
