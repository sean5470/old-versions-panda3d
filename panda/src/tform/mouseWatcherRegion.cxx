// Filename: mouseWatcherRegion.cxx
// Created by:  drose (13Jul00)
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

#include "mouseWatcherRegion.h"

#include <indent.h>


TypeHandle MouseWatcherRegion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
output(ostream &out) const {
  out << get_name() << " lrbt = " << _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_name() << " lrbt = " << _frame << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
enter(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
exit(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
press(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
release(const MouseWatcherParameter &) {
}
