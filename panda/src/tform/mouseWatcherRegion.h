// Filename: mouseWatcherRegion.h
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

#ifndef MOUSEWATCHERREGION_H
#define MOUSEWATCHERREGION_H

#include "pandabase.h"

#include "namable.h"
#include "typedReferenceCount.h"
#include "luse.h"
#include "buttonHandle.h"
#include "modifierButtons.h"

class MouseWatcherParameter;

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcherRegion
// Description : This is the class that defines a rectangular region
//               on the screen for the MouseWatcher.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseWatcherRegion : public TypedReferenceCount, public Namable {
PUBLISHED:
  INLINE MouseWatcherRegion(const string &name, float left, float right,
                            float bottom, float top);
  INLINE MouseWatcherRegion(const string &name, const LVecBase4f &frame);

  INLINE void set_frame(float left, float right, float bottom, float top);
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE const LVecBase4f &get_frame() const;
  INLINE float get_area() const;

  INLINE void set_sort(int sort);
  INLINE int get_sort() const;

  INLINE void set_active(bool active);
  INLINE bool get_active() const;

  INLINE void set_suppress_below(bool suppress_below);
  INLINE bool get_suppress_below() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE bool operator < (const MouseWatcherRegion &other) const;

  virtual void enter(const MouseWatcherParameter &param);
  virtual void exit(const MouseWatcherParameter &param);
  virtual void press(const MouseWatcherParameter &param);
  virtual void release(const MouseWatcherParameter &param);

private:
  LVecBase4f _frame;
  float _area;
  int _sort;

  bool _active;
  bool _suppress_below;

  ModifierButtons _mods;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MouseWatcherRegion",
                  TypedReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const MouseWatcherRegion &region) {
  region.output(out);
  return out;
}

#include "mouseWatcherRegion.I"

#endif
