// Filename: config_grutil.cxx
// Created by:  drose (24May00)
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

#include "config_grutil.h"
#include "frameRateMeter.h"
#include "sceneGraphAnalyzerMeter.h"
#include "meshDrawer.h"
#include "meshDrawer2D.h"
#include "geoMipTerrain.h"
#include "movieTexture.h"
#include "pandaSystem.h"
#include "texturePool.h"
#include "nodeVertexTransform.h"
#include "rigidBodyCombiner.h"
#include "pipeOcclusionCullTraverser.h"

#include "dconfig.h"

Configure(config_grutil);
NotifyCategoryDef(grutil, "");

ConfigureFn(config_grutil) {
  init_libgrutil();
}

ConfigVariableDouble frame_rate_meter_update_interval
("frame-rate-meter-update-interval", 1.5);

ConfigVariableString frame_rate_meter_text_pattern
("frame-rate-meter-text-pattern", "%0.1f fps");

ConfigVariableInt frame_rate_meter_layer_sort
("frame-rate-meter-layer-sort", 1000);

ConfigVariableDouble frame_rate_meter_scale
("frame-rate-meter-scale", 0.05);

ConfigVariableDouble frame_rate_meter_side_margins
("frame-rate-meter-side-margins", 0.5);

ConfigVariableDouble scene_graph_analyzer_meter_update_interval
("scene-graph-analyzer-meter-update-interval", 2.0);

ConfigVariableInt scene_graph_analyzer_meter_layer_sort
("scene-graph-analyzer-meter-layer-sort", 1000);

ConfigVariableDouble scene_graph_analyzer_meter_scale
("scene-graph-analyzer-meter-scale", 0.05);

ConfigVariableDouble scene_graph_analyzer_meter_side_margins
("scene-graph-analyzer-meter-side-margins", 0.5);

ConfigVariableBool pfm_force_littleendian
("pfm-force-littleendian", false,
 PRC_DESC("This forces a pfm file to be read as a sequence of little-endian "
          "floats, even if its scale factor is given as a positive number."));

ConfigVariableBool pfm_reverse_dimensions
("pfm-reverse-dimensions", false,
 PRC_DESC("Understands that the width and height of a pfm file are given "
          "backwards, in the form height width instead of width height, "
          "on input.  Does not affect output, which is always written width height."));

ConfigVariableInt pfm_vis_max_vertices
("pfm-vis-max-vertices", 65535,
 PRC_DESC("Specifies the maximum number of vertex entries that may appear in "
          "a single generated mesh.  If the mesh would require more than that, "
          "the mesh is subdivided into smaller pieces."));

ConfigVariableInt pfm_vis_max_indices
("pfm-vis-max-indices", 1048576,
 PRC_DESC("Specifies the maximum number of vertex references that may appear in "
          "a single generated mesh.  If the mesh would require more than that, "
          "the mesh is subdivided into smaller pieces."));

////////////////////////////////////////////////////////////////////
//     Function: init_libgrutil
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libgrutil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  FrameRateMeter::init_type();
  MeshDrawer::init_type();
  MeshDrawer2D::init_type();
  GeoMipTerrain::init_type();
  NodeVertexTransform::init_type();
  RigidBodyCombiner::init_type();
  PipeOcclusionCullTraverser::init_type();
  SceneGraphAnalyzerMeter::init_type();

#ifdef HAVE_AUDIO
  MovieTexture::init_type();
  MovieTexture::register_with_read_factory();
#endif  // HAVE_AUDIO

#ifdef HAVE_FFMPEG
  MovieTexture::init_type();
  MovieTexture::register_with_read_factory();

  TexturePool *ts = TexturePool::get_global_ptr();
  ts->register_texture_type(MovieTexture::make_texture, "avi mov mpg mpeg mp4 wmv asf flv nut ogm mkv");
#endif  // HAVE_FFMPEG
}

