// Filename: chunkedStreamBuf.cxx
// Created by:  drose (25Sep02)
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

#include "chunkedStreamBuf.h"
#include <ctype.h>

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ChunkedStreamBuf::
ChunkedStreamBuf() {
  _chunk_remaining = 0;
  _done = true;

#ifdef WIN32_VC
  // In spite of the claims of the MSDN Library to the contrary,
  // Windows doesn't seem to provide an allocate() function, so we'll
  // do it by hand.
  char *buf = new char[4096];
  char *ebuf = buf + 4096;
  setg(buf, ebuf, ebuf);
  setp(buf, ebuf);

#else
  allocate();
  setg(base(), ebuf(), ebuf());
  setp(base(), ebuf());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ChunkedStreamBuf::
~ChunkedStreamBuf() {
  close_read();
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::open_read
//       Access: Public
//  Description: If the document pointer is non-NULL, it will be
//               updated with the length of the file as it is derived
//               from the chunked encoding.
////////////////////////////////////////////////////////////////////
void ChunkedStreamBuf::
open_read(BioStreamPtr *source, HTTPChannel *doc) {
  _source = source;
  nassertv(!_source.is_null());
  _chunk_remaining = 0;
  _done = false;
  _doc = doc;

  if (_doc != (HTTPChannel *)NULL) {
    _read_index = doc->_read_index;
    _doc->_file_size = 0;

    // Read a little bit from the file to get the first chunk (and
    // therefore the file size, or at least the size of the first
    // chunk).
    underflow();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::close_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ChunkedStreamBuf::
close_read() {
  _source.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int ChunkedStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    size_t num_bytes = buffer_size;
    size_t read_count = read_chars(gptr(), buffer_size);

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}


////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::read_chars
//       Access: Private
//  Description: Gets some characters from the source stream.
////////////////////////////////////////////////////////////////////
size_t ChunkedStreamBuf::
read_chars(char *start, size_t length) {
  nassertr(!_source.is_null(), 0);
  if (_done) {
    return 0;
  }

  if (_chunk_remaining != 0) {
    // Extract some of the bytes remaining in the chunk.
    length = min(length, _chunk_remaining);
    (*_source)->read(start, length);
    size_t read_count = (*_source)->gcount();
    _chunk_remaining -= read_count;

    if (read_count == 0 && (*_source)->is_closed()) {
      // Whoops, the socket closed while we were downloading.
      if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
        _doc->_state = HTTPChannel::S_failure;
      }
    }

    return read_count;
  }

  // Read the next chunk.
  string line;
  bool got_line = http_getline(line);
  while (got_line && line.empty()) {
    // Skip blank lines.  There really should be exactly one blank
    // line, but who's counting?  It's tricky to count and maintain
    // reentry for nonblocking I/O.
    got_line = http_getline(line);
  }
  if (!got_line) {
    // EOF (or data unavailable) while trying to read the chunk size.
    if ((*_source)->is_closed()) {
      // Whoops, the socket closed while we were downloading.
      if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
        _doc->_state = HTTPChannel::S_failure;
      }
    }
    return 0;
  }
  size_t chunk_size = (size_t)strtol(line.c_str(), NULL, 16);
  if (chunk_size == 0) {
    // Last chunk; we're done.
    _done = true;
    if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
      _doc->finished_body(true);
    }
    return 0;
  }

  if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
    _doc->_file_size += chunk_size;
  }

  _chunk_remaining = chunk_size;
  return read_chars(start, length);
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::http_getline
//       Access: Private
//  Description: Reads a single line from the stream.  Returns
//               true if the line is successfully retrieved, or false
//               if a complete line has not yet been received or if
//               the connection has been closed.
////////////////////////////////////////////////////////////////////
bool ChunkedStreamBuf::
http_getline(string &str) {
  nassertr(!_source.is_null(), false);
  int ch = (*_source)->get();
  while (!(*_source)->eof() && !(*_source)->fail()) {
    switch (ch) {
    case '\n':
      // end-of-line character, we're done.
      str = _working_getline;
      _working_getline = string();
      {
        // Trim trailing whitespace.  We're not required to do this per the
        // HTTP spec, but let's be generous.
        size_t p = str.length();
        while (p > 0 && isspace(str[p - 1])) {
          --p;
        }
        str = str.substr(0, p);
      }

      return true;

    case '\r':
      // Ignore CR characters.
      break;

    default:
      _working_getline += (char)ch;
    }
    ch = (*_source)->get();
  }

  return false;
}

#endif  // HAVE_SSL
