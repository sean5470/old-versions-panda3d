// Filename: depthTestAttrib.h
// Created by:  drose (04Mar02)
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

#ifndef DEPTHTESTATTRIB_H
#define DEPTHTESTATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : DepthTestAttrib
// Description : Enables or disables writing to the depth buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DepthTestAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_none,             // No depth test; may still write to depth buffer.
    M_never,            // Never draw.
    M_less,             // incoming < stored
    M_equal,            // incoming == stored
    M_less_equal,       // incoming <= stored
    M_greater,          // incoming > stored
    M_not_equal,        // incoming != stored
    M_greater_equal,    // incoming >= stored
    M_always            // Always draw.  Same effect as none, more expensive.
  };

private:
  INLINE DepthTestAttrib(Mode mode = M_less);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);

  INLINE Mode get_mode() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Mode _mode;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "DepthTestAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "depthTestAttrib.I"

#endif

