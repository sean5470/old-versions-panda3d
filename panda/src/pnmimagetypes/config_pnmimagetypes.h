// Filename: config_pnmimagetypes.h
// Created by:  drose (17Jun00)
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

#ifndef CONFIG_PNMIMAGETYPES_H
#define CONFIG_PNMIMAGETYPES_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableInt.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "sgi.h"

NotifyCategoryDecl(pnmimage_sgi, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_alias, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_tiff, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_tga, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_img, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_soft, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_bmp, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_jpg, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_png, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);
NotifyCategoryDecl(pnmimage_pnm, EXPCL_PANDA_PNMIMAGETYPES, EXPTP_PANDA_PNMIMAGETYPES);

enum SGIStorageType {
  SST_rle = STORAGE_RLE,
  SST_verbatim = STORAGE_VERBATIM,
};

EXPCL_PANDA_PNMIMAGETYPES ostream &operator << (ostream &out, SGIStorageType sst);
EXPCL_PANDA_PNMIMAGETYPES istream &operator >> (istream &in, SGIStorageType &sst);

extern ConfigVariableEnum<SGIStorageType> sgi_storage_type;
extern ConfigVariableString sgi_imagename;
extern ConfigVariableBool tga_rle;
extern ConfigVariableBool tga_colormap;
extern ConfigVariableBool tga_grayscale;

extern ConfigVariableInt jpeg_quality;

extern ConfigVariableInt bmp_bpp;

enum IMGHeaderType {
  IHT_none,
  IHT_short,
  IHT_long,
};

EXPCL_PANDA_PNMIMAGETYPES ostream &operator << (ostream &out, IMGHeaderType iht);
EXPCL_PANDA_PNMIMAGETYPES istream &operator >> (istream &in, IMGHeaderType &iht);

extern ConfigVariableEnum<IMGHeaderType> img_header_type;
extern ConfigVariableInt img_size;

extern EXPCL_PANDA_PNMIMAGETYPES void init_libpnmimagetypes();

#endif
