// Filename: config_wgldisplay.h
// Created by:  mike (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_WGLDISPLAY_H__
#define __CONFIG_WGLDISPLAY_H__

#include <pandabase.h>
#include <filename.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(wgldisplay, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern Filename get_icon_filename();

extern EXPCL_PANDAGL void init_libwgldisplay();

#endif /* __CONFIG_WGLDISPLAY_H__ */
