// Filename: winGraphicsPipe.cxx
// Created by:  drose (20Dec02)
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

#include "winGraphicsPipe.h"
#include "config_windisplay.h"
#include "displaySearchParameters.h"

TypeHandle WinGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WinGraphicsPipe::
WinGraphicsPipe() {
  _supported_types = OT_window | OT_fullscreen_window;

  // these fns arent defined on win95, so get dynamic ptrs to them
  // to avoid ugly DLL loader failures on w95
  _pfnTrackMouseEvent = NULL;

  _hUser32 = (HINSTANCE)LoadLibrary("user32.dll");
  if (_hUser32 != NULL) {
    _pfnTrackMouseEvent = 
      (PFN_TRACKMOUSEEVENT)GetProcAddress(_hUser32, "TrackMouseEvent");
  }

  int dx7_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information);
  int dx8_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information);
  int dx9_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information);

  DisplaySearchParameters display_search_parameters;

  // DX7
  if (dx7_display_information (display_search_parameters, _display_information)) {
  
  }
  
  // try DX9 first
  if (dx9_display_information (display_search_parameters, _display_information)) {
  
  }
  else {
    // DX9 failed, try DX8
    if (dx8_display_information (display_search_parameters, _display_information)) {

    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
WinGraphicsPipe::
~WinGraphicsPipe() {
  if (_hUser32 != NULL) {
    FreeLibrary(_hUser32);
    _hUser32 = NULL;
  }
}

bool MyGetProcAddr(HINSTANCE hDLL, FARPROC *pFn, const char *szExportedFnName) {
  *pFn = (FARPROC) GetProcAddress(hDLL, szExportedFnName);
  if (*pFn == NULL) {
    windisplay_cat.error() << "GetProcAddr failed for " << szExportedFnName << ", error=" << GetLastError() <<endl;
    return false;
  }
  return true;
}

bool MyLoadLib(HINSTANCE &hDLL, const char *DLLname) {
  hDLL = LoadLibrary(DLLname);
  if(hDLL == NULL) {
    windisplay_cat.error() << "LoadLibrary failed for " << DLLname << ", error=" << GetLastError() <<endl;
    return false;
  }
  return true;
}
