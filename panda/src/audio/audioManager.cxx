// Filename: audioManager.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
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

#include "config_audio.h"
#include "audioManager.h"

#include "nullAudioManager.h"

#include "load_dso.h"


TypeHandle AudioManager::_type_handle;


namespace {
  PT(AudioManager) create_NullAudioManger() {
    audio_debug("create_NullAudioManger()");
    return new NullAudioManager();
  }
}

Create_AudioManager_proc* AudioManager::_create_AudioManager
    =create_NullAudioManger;

void AudioManager::
register_AudioManager_creator(Create_AudioManager_proc* proc) {
  nassertv(_create_AudioManager==create_NullAudioManger);
  _create_AudioManager=proc;
}



// Factory method for getting a platform specific AudioManager:
PT(AudioManager) AudioManager::
create_AudioManager() {
  audio_debug("create_AudioManager()\n  audio_library_name=\""
      <<*audio_library_name<<"\"");
  static int lib_load;
  if (!lib_load) {
    lib_load=1;
    if (!audio_library_name->empty() && !((*audio_library_name) == "null")) {
      Filename dl_name = Filename::dso_filename(
          "lib"+*audio_library_name+".so");
      dl_name.to_os_specific();
      audio_debug("  dl_name=\""<<dl_name<<"\"");
      void* lib = load_dso(dl_name);
      if (!lib) {
        audio_error("  LoadLibrary() failed, will use NullAudioManager");
        audio_error("    "<<load_dso_error());
        nassertr(_create_AudioManager==create_NullAudioManger, 0);
      } else {
        // ...the library will register itself with the AudioManager.
        #if defined(DWORD) && defined(WIN32) && !defined(NDEBUG) //[
          const int bufLen=256;
          char path[bufLen];
          DWORD count = GetModuleFileName((HMODULE)lib, path, bufLen);
          if (count) {
            audio_debug("  GetModuleFileName() \""<<path<<"\"");
          } else {
            audio_debug("  GetModuleFileName() failed.");
          }
        #endif //]
      }
    }
  }
  PT(AudioManager) am = (*_create_AudioManager)();
  if (!am->is_valid()) {
    am = create_NullAudioManger();
  }
  return am;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_null_sound
//       Access: Public
//  Description: Returns a special NullAudioSound object that has all
//               the interface of a normal sound object, but plays no
//               sound.  This same object may also be returned by
//               get_sound() if it fails.
////////////////////////////////////////////////////////////////////
PT(AudioSound) AudioManager::
get_null_sound() {
  if (_null_sound == (AudioSound *)NULL) {
    _null_sound = new NullAudioSound;
  }
  return _null_sound;
}

