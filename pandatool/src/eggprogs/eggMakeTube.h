// Filename: eggMakeTube.h
// Created by:  drose (01Oct03)
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

#ifndef EGGMAKETUBE_H
#define EGGMAKETUBE_H

#include "pandatoolbase.h"

#include "eggMakeSomething.h"

class EggGroup;
class EggVertexPool;
class EggVertex;

////////////////////////////////////////////////////////////////////
//       Class : EggMakeTube
// Description : A program to generate an egg file representing a tube
//               model, similar in shape to a CollisionTube.
////////////////////////////////////////////////////////////////////
class EggMakeTube : public EggMakeSomething {
public:
  EggMakeTube();

  void run();

private:
  EggVertex *calc_sphere1_vertex(int ri, int si);
  EggVertex *calc_sphere2_vertex(int ri, int si);
  EggVertex *calc_tube_vertex(int ri, int si);
  void add_polygon(EggVertex *a, EggVertex *b, EggVertex *c, EggVertex *d);

private:
  double _point_a[3];
  double _point_b[3];
  double _radius;
  int _num_slices;
  int _num_crings;
  int _num_trings;

  double _length;
  EggGroup *_group;
  EggVertexPool *_vpool;
};

#endif

