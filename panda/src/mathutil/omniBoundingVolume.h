// Filename: omniBoundingVolume.h
// Created by:  drose (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef OMNIBOUNDINGVOLUME_H
#define OMNIBOUNDINGVOLUME_H

#include <pandabase.h>

#include "geometricBoundingVolume.h"

///////////////////////////////////////////////////////////////////
// 	 Class : OmniBoundingVolume
// Description : This is a special kind of GeometricBoundingVolume
//               that fills all of space.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OmniBoundingVolume : public GeometricBoundingVolume {
PUBLISHED:
  INLINE OmniBoundingVolume();

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3f get_approx_center() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
			    const BoundingVolume **first,
			    const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3f &point);
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);

  virtual bool around_points(const LPoint3f *first,
			     const LPoint3f *last);
  virtual bool around_spheres(const BoundingVolume **first,
			      const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
				  const BoundingVolume **last);

  virtual int contains_point(const LPoint3f &point) const;
  virtual int contains_lineseg(const LPoint3f &a, const LPoint3f &b) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "OmniBoundingVolume",
		  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingHexahedron;
};

#include "omniBoundingVolume.I"

#endif
