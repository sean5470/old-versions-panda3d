// Filename: audio_linux_traits.h
// Created by:  cary (02Oct00)
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

// yes, this needs to be outside the ifdef protection
#include "audio_trait.h"

#ifdef AUDIO_USE_LINUX

#ifndef __AUDIO_LINUX_TRAITS_H__
#define __AUDIO_LINUX_TRAITS_H__

#include "config_audio.h"

#ifndef HAVE_DEFINED_BYTE
typedef unsigned char byte;
#define HAVE_DEFINED_BYTE
#endif /* HAVE_DEFINED_BYTE */

extern byte* zero_buffer;

class EXPCL_PANDA Buffer {
private:
  byte* _data;
  unsigned long _size;
  unsigned long _pos;
  bool _done;

  INLINE unsigned long buffer_min(unsigned long a, unsigned long b) {
    return (a<b)?a:b;
  }
public:
  INLINE Buffer(byte* data, unsigned long size) : _data(data), _size(size),
                                                  _pos(0), _done(false) {}
  INLINE byte* get_buffer(byte* buf) {
    unsigned long copy_size = buffer_min(_size-_pos, audio_buffer_size);
    unsigned long residue = audio_buffer_size - copy_size;
    memcpy(buf, &_data[_pos], copy_size);
    _pos += copy_size;
    if (residue != 0) {
      _pos -= _size;
      memcpy(&buf[copy_size], zero_buffer, residue);
      _done = true;
    }
    return buf;
  }
  INLINE bool is_done() const {
    return _done;
  }
  INLINE unsigned long get_size() const {
    return _size;
  }
  INLINE void reset(unsigned long p = 0) {
    _done = false;
    _pos = p;
  }
};

class LinuxSamplePlaying;

class EXPCL_PANDA LinuxSample : public AudioTraits::SoundClass {
private:
  byte* _data;
  unsigned long _size;
public:
  INLINE LinuxSample(byte*, unsigned long);
  virtual ~LinuxSample();

  virtual float length() const;
  virtual AudioTraits::PlayingClass* get_state() const;
  virtual AudioTraits::PlayerClass* get_player() const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate() const;
public:
  // used by the loader
  static LinuxSample* load_raw(byte*, unsigned long);
  // used by the players
  INLINE byte* get_data() const;
  INLINE unsigned long get_size() const;
};

class EXPCL_PANDA LinuxMusic : public AudioTraits::SoundClass {
public:
  INLINE LinuxMusic();
  virtual ~LinuxMusic();

  virtual float length() const;
  virtual AudioTraits::PlayingClass* get_state() const;
  virtual AudioTraits::PlayerClass* get_player() const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate() const;
};

class EXPCL_PANDA LinuxSamplePlaying : public AudioTraits::PlayingClass {
  Buffer* _buff;
public:
  INLINE LinuxSamplePlaying(AudioTraits::SoundClass*);
  virtual ~LinuxSamplePlaying();

  virtual AudioTraits::PlayingClass::PlayingStatus status();
public:
  INLINE Buffer* get_data();
  static void destroy(AudioTraits::PlayingClass*);
};

class EXPCL_PANDA LinuxMusicPlaying : public AudioTraits::PlayingClass {
public:
  INLINE LinuxMusicPlaying(AudioTraits::SoundClass*);
  virtual ~LinuxMusicPlaying();

  virtual AudioTraits::PlayingClass::PlayingStatus status();
  static void destroy(AudioTraits::PlayingClass*);
};

class EXPCL_PANDA LinuxSamplePlayer : public AudioTraits::PlayerClass {
public:
  INLINE LinuxSamplePlayer();
  virtual ~LinuxSamplePlayer();

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static LinuxSamplePlayer* get_instance();
private:
  static LinuxSamplePlayer* _global_instance;
};

class EXPCL_PANDA LinuxMusicPlayer : public AudioTraits::PlayerClass {
public:
  INLINE LinuxMusicPlayer();
  virtual ~LinuxMusicPlayer();

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static LinuxMusicPlayer* get_instance();
private:
  static LinuxMusicPlayer* _global_instance;
};

#include "audio_linux_traits.I"

#endif /* __AUDIO_LINUX_TRAITS_H__ */
#endif /* AUDIO_USE_LINUX */
