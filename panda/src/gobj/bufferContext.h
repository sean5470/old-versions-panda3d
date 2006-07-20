// Filename: bufferContext.h
// Created by:  drose (16Mar06)
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

#ifndef BUFFERCONTEXT_H
#define BUFFERCONTEXT_H

#include "pandabase.h"

#include "savedContext.h"
#include "updateSeq.h"
#include "linkedListNode.h"
#include "bufferContextChain.h"
#include "bufferResidencyTracker.h"

class PreparedGraphicsObjects;

////////////////////////////////////////////////////////////////////
//       Class : BufferContext
// Description : This is a base class for those kinds of SavedContexts
//               that occupy an easily-measured (and substantial)
//               number of bytes in the video card's frame buffer
//               memory or AGP memory.  At the present, this includes
//               most of the SavedContext types: VertexBufferContext
//               and IndexBufferContext, as well as TextureContext.
//
//               This class provides methods for tracking the video
//               memory utilization, as well as residency of each
//               object, via PStats.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BufferContext : public SavedContext, private LinkedListNode {
public:
  BufferContext(BufferResidencyTracker *residency);
  virtual ~BufferContext();

  INLINE size_t get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;

  INLINE void set_active(bool flag);
  INLINE void set_resident(bool flag);

  INLINE BufferContext *get_next() const;

  INLINE void update_data_size_bytes(size_t new_data_size_bytes);
  INLINE void update_modified(UpdateSeq new_modified);

private:
  void set_owning_chain(BufferContextChain *chain);

private:
  BufferResidencyTracker *_residency;
  int _residency_state;

  size_t _data_size_bytes;
  UpdateSeq _modified;
  BufferContextChain *_owning_chain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "BufferContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
  friend class BufferResidencyTracker;
  friend class BufferContextChain;
};

#include "bufferContext.I"

#endif

