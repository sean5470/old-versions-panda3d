// Filename: fakestringstream.h
// Created by:  cary (04Feb99)
// 
////////////////////////////////////////////////////////////////////

#ifndef FAKESTRINGSTREAM_H
#define FAKESTRINGSTREAM_H

#include <strstream.h>
#include <string.h>
#include <string>

#ifdef HAVE_NAMESPACE
using namespace std;
#endif

class fake_istream_buffer {
public:
  fake_istream_buffer() {
    _len = 0;
    _str = "";
  }
  fake_istream_buffer(const string &source) {
    _len = source.length();
    if (_len == 0) {
      _str = "";
    } else {
      _str = new char[_len];
      memcpy(_str, source.data(), _len);
    }
  }
  ~fake_istream_buffer() {
    if (_len != 0) {
      delete[] _str;
    }
  }

  int _len;
  char *_str;
};

class istringstream : public fake_istream_buffer, public istrstream {
public:
  istringstream(const string &input) : 
    fake_istream_buffer(input),
    istrstream(_str, _len) { }
};

class ostringstream : public ostrstream {
public:
  string str() {
    // We must capture the length before we take the str().
    int length = pcount();
    char *s = ostrstream::str();
    string result(s, length);
    delete[] s;
    return result;
  }
};

class stringstream : public fake_istream_buffer, public strstream {
public:
  stringstream() : strstream() { 
    _owns_str = true;
  }
  stringstream(const string &input) : 
    fake_istream_buffer(input),
    strstream(_str, _len, ios::in) 
  {
    _owns_str = false;
  }
  string str() {
    char *s = strstream::str();
    string result(s);
    if (_owns_str) {
      delete[] s;
    }
    return result;
  }

private:
  bool _owns_str;
};

#endif
