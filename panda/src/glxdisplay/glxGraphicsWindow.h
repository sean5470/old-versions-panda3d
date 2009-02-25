// Filename: glxGraphicsWindow.h
// Created by:  mike (09Jan97)
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

#ifndef GLXGRAPHICSWINDOW_H
#define GLXGRAPHICSWINDOW_H

#include "pandabase.h"

#include "glxGraphicsPipe.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsWindow
// Description : An interface to the glx system for managing GL
//               windows under X.
////////////////////////////////////////////////////////////////////
class glxGraphicsWindow : public GraphicsWindow {
public:
  glxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe, 
                    const string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~glxGraphicsWindow();

  virtual bool move_pointer(int device, int x, int y);
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void begin_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void set_wm_properties(const WindowProperties &properties,
                         bool already_mapped);

#ifdef HAVE_GLXFBCONFIG
  void setup_colormap(GLXFBConfig fbconfig);
#endif  // HAVE_GLXFBCONFIG
  void setup_colormap(XVisualInfo *visual);
  void handle_keystroke(XKeyEvent &event);
  void handle_keypress(XKeyEvent &event);
  void handle_keyrelease(XKeyEvent &event);

  ButtonHandle get_button(XKeyEvent &key_event, bool allow_shift);
  ButtonHandle map_button(KeySym key);
  ButtonHandle get_mouse_button(XButtonEvent &button_event);

  static Bool check_event(Display *display, XEvent *event, char *arg);

  void open_raw_mice();
  void poll_raw_mice();
  
private:
  Display *_display;
  int _screen;
  Window _xwindow;
  Colormap _colormap;
  XIC _ic;

  long _event_mask;
  bool _awaiting_configure;
  Atom _wm_delete_window;
  Atom _net_wm_window_type;
  Atom _net_wm_window_type_splash;
  Atom _net_wm_window_type_fullscreen;
  Atom _net_wm_state;
  Atom _net_wm_state_fullscreen;
  Atom _net_wm_state_above;
  Atom _net_wm_state_below;
  Atom _net_wm_state_add;
  Atom _net_wm_state_remove;

  struct MouseDeviceInfo {
    int    _fd;
    int    _input_device_index;
    string _io_buffer;
  };
  pvector<MouseDeviceInfo> _mouse_device_info;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "glxGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsWindow.I"

#endif
