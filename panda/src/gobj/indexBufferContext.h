// Filename: indexBufferContext.h
// Created by:  drose (17Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef INDEXBUFFERCONTEXT_H
#define INDEXBUFFERCONTEXT_H

#include "pandabase.h"

#include "bufferContext.h"
#include "geomPrimitive.h"
#include "preparedGraphicsObjects.h"

////////////////////////////////////////////////////////////////////
//       Class : IndexBufferContext
// Description : This is a special class object that holds all the
//               information returned by a particular GSG to indicate
//               the vertex data array's internal context identifier.
//
//               This allows the GSG to cache the vertex data array in
//               whatever way makes sense.  For instance, DirectX can
//               allocate a vertex buffer for the array.  OpenGL can
//               create a buffer object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ IndexBufferContext : public BufferContext {
public:
  INLINE IndexBufferContext(PreparedGraphicsObjects *pgo, GeomPrimitive *data);

PUBLISHED:
  INLINE GeomPrimitive *get_data() const;

  INLINE bool changed_size(const GeomPrimitivePipelineReader *reader) const;
  INLINE bool changed_usage_hint(const GeomPrimitivePipelineReader *reader) const;
  INLINE bool was_modified(const GeomPrimitivePipelineReader *reader) const;

public:
  INLINE void mark_loaded(const GeomPrimitivePipelineReader *reader);

private:
  // This cannot be a PT(GeomPrimitive), because the data and
  // the GSG both own their IndexBufferContexts!  That would create a
  // circular reference count.
  GeomPrimitive *_data;
  GeomEnums::UsageHint _usage_hint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BufferContext::init_type();
    register_type(_type_handle, "IndexBufferContext",
                  BufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

#include "indexBufferContext.I"

#endif

