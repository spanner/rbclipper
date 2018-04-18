/*
 * Clipper Ruby Bindings
 * Copyright 2010 Mike Owens <http://mike.filespanker.com/>
 * Changed by Dag Rende for Clipper after 2.9
 *
 * Released under the same terms as Clipper.
 *
 */

#include "clipper.hpp"
#include <ruby.h>

#ifndef DBL2NUM
# define DBL2NUM rb_float_new
#endif

using namespace std;
using namespace ClipperLib;

static ID id_even_odd;
static ID id_non_zero;
static ID id_positive;
static ID id_negative;
static ID id_polygons;
static ID id_ex_polygons;
static ID id_jtSquare;
static ID id_jtMiter;
static ID id_jtRound;
static ID id_etClosedPolygon;
static ID id_etClosedLine;
static ID id_etOpenButt;
static ID id_etOpenSquare;
static ID id_etOpenRound;

static inline Clipper*
XCLIPPER(VALUE x)
{
  Clipper* clipper;
  Data_Get_Struct(x, Clipper, clipper);
  return clipper;
}

static inline ClipperOffset*
XCLIPPEROFFSET(VALUE x)
{
  ClipperOffset* clipperOffset;
  Data_Get_Struct(x, ClipperOffset, clipperOffset);
  return clipperOffset;
}

static inline PolyFillType
sym_to_filltype(VALUE sym)
{
  ID inp = rb_to_id(sym);

  if (inp == id_even_odd) {
    return pftEvenOdd;
  } else if (inp == id_non_zero) {
    return pftNonZero;
  } else if (inp == id_positive) {
    return pftPositive;
  } else if (inp == id_negative) {
    return pftNegative;
  }

  rb_raise(rb_eArgError, "%s", "Expected :even_odd, :non_zero, :positive or :negative");
}

static inline EndType
sym_to_endtype(VALUE sym)
{
  ID inp = rb_to_id(sym);

  if (inp == id_etClosedPolygon) {
    return etClosedPolygon;
  } else if (inp == id_etClosedLine) {
    return etClosedLine;
  } else if (inp == id_etOpenButt) {
    return etOpenButt;
  } else if (inp == id_etOpenSquare) {
    return etOpenSquare;
  } else if (inp == id_etOpenRound) {
    return etOpenRound;
  }

  rb_raise(rb_eArgError, "%s", "Expected :etClosedPolygon, :etClosedLine or :etOpenButt or :etOpenSquare or :etOpenRound");
}

static inline JoinType
sym_to_jointype(VALUE sym)
{
  ID inp = rb_to_id(sym);

  if (inp == id_jtSquare) {
    return jtSquare;
  } else if (inp == id_jtMiter) {
    return jtMiter;
  } else if (inp == id_jtRound) {
    return jtRound;
  }

  rb_raise(rb_eArgError, "%s", "Expected :jtSquare, :jtMiter or :jtRound");
}

extern "C" {

static inline IntPoint
xy_to_intpoint(VALUE px, VALUE py, double multiplier) {
  return IntPoint((long64)(NUM2DBL(px) * multiplier), (long64)(NUM2DBL(py) * multiplier));
}

static void
ary_to_polygon(VALUE ary, ClipperLib::Path* poly, double multiplier)
{
  const char* earg =
    "Paths have format: [[p0_x, p0_y], [p1_x, p1_y], ...]";

  Check_Type(ary, T_ARRAY);

  for(long i = 0; i != RARRAY_LEN(ary); i++) {
    VALUE sub = rb_ary_entry(ary, i);
    Check_Type(sub, T_ARRAY);

    if(RARRAY_LEN(sub) != 2) {
      rb_raise(rb_eArgError, "%s", earg);
    }

    VALUE px = rb_ary_entry(sub, 0);
    VALUE py = rb_ary_entry(sub, 1);
    poly->push_back(IntPoint((long64)(NUM2DBL(px) * multiplier), (long64)(NUM2DBL(py) * multiplier)));
  }
}

static void
arys_to_polygons(VALUE polygonsValue, ClipperLib::Paths* polygons, const double multiplier)
{
  for(long i = 0; i != RARRAY_LEN(polygonsValue); i++) {
    VALUE sub = rb_ary_entry(polygonsValue, i);
    Check_Type(sub, T_ARRAY);

    ClipperLib::Path tmp;
    ary_to_polygon(sub, &tmp, multiplier);
    polygons->push_back(tmp);
  }
}

static void
rbclipper_free(void* ptr)
{
  delete (Clipper*) ptr;
}

static void
rbclipperoffset_free(void* ptr)
{
  delete (ClipperOffset*) ptr;
}

static VALUE
rbclipper_new(VALUE klass)
{
  Clipper* ptr = new Clipper;
  VALUE r = Data_Wrap_Struct(klass, 0, rbclipper_free, ptr);
  rb_obj_call_init(r, 0, 0);
  rb_iv_set(r, "@multiplier", INT2NUM(1048576));
  return r;
}

static VALUE
rbclipperoffset_new(VALUE klass)
{
  ClipperOffset* ptr = new ClipperOffset;
  VALUE r = Data_Wrap_Struct(klass, 0, rbclipperoffset_free, ptr);
  rb_obj_call_init(r, 0, 0);
  rb_iv_set(r, "@multiplier", INT2NUM(1048576));
  return r;
}

static VALUE
rbclipper_add_polygon_internal(VALUE self, VALUE polygon,
                               PolyType polytype, bool closed = true)
{
  ClipperLib::Path tmp;
  double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
  ary_to_polygon(polygon, &tmp, multiplier);
  XCLIPPER(self)->AddPath(tmp, polytype, closed);
  return Qnil;
}

static VALUE
rbclipper_add_polygons_internal(VALUE self, VALUE polygonsValue, PolyType polytype, bool closed = true) {
  Paths tmp;
  const double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
  arys_to_polygons(polygonsValue, &tmp, multiplier);
  XCLIPPER(self)->AddPaths(tmp, polytype, closed);
  return Qnil;
}

static VALUE
rbclipper_minkowski_sum(VALUE self, VALUE patternValue, VALUE pathsValue, bool isClosed = true) {
  Paths paths;
  const double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
  arys_to_polygons(pathsValue, &paths, multiplier);

  Path pattern;
  ary_to_polygon(patternValue, &pattern, multiplier);

  Paths solution;
  ClipperLib::MinkowskiSum(pattern, paths, solution, isClosed);

  const double inv_multiplier = 1.0 / multiplier;

  VALUE r = rb_ary_new();
  for(Paths::iterator i = solution.begin(); i != solution.end(); ++i) {
    VALUE sub = rb_ary_new();
    for(Path::iterator p = i->begin(); p != i->end(); ++p) {
        rb_ary_push(sub, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
    }
    rb_ary_push(r, sub);
  }
  return r;
}

static VALUE
rbclipper_minkowski_diff(VALUE self, VALUE poly1v, VALUE poly2v) {
  const double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
  Path poly1;
  ary_to_polygon(poly1v, &poly1, multiplier);
  Path poly2;
  ary_to_polygon(poly2v, &poly2, multiplier);

  Paths solution;
  ClipperLib::MinkowskiDiff(poly1, poly2, solution);

  const double inv_multiplier = 1.0 / multiplier;

  VALUE r = rb_ary_new();
  for(Paths::iterator i = solution.begin(); i != solution.end(); ++i) {
    VALUE sub = rb_ary_new();
    for(Path::iterator p = i->begin(); p != i->end(); ++p) {
        rb_ary_push(sub, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
    }
    rb_ary_push(r, sub);
  }
  return r;
}

static VALUE
rbclipper_add_subject_polyline(VALUE self, VALUE polyline)
{
  return rbclipper_add_polygon_internal(self, polyline, ptSubject, false);
}

static VALUE
rbclipper_add_subject_polylines(VALUE self, VALUE polylines)
{
  return rbclipper_add_polygons_internal(self, polylines, ptSubject, false);
}

static VALUE
rbclipper_add_subject_polygon(VALUE self, VALUE polygon)
{
  return rbclipper_add_polygon_internal(self, polygon, ptSubject);
}

static VALUE
rbclipper_add_clip_polygon(VALUE self, VALUE polygon)
{
  return rbclipper_add_polygon_internal(self, polygon, ptClip);
}

static VALUE
rbclipper_add_subject_polygons(VALUE self, VALUE polygons)
{
  return rbclipper_add_polygons_internal(self, polygons, ptSubject);
}

static VALUE
rbclipper_add_clip_polygons(VALUE self, VALUE polygons)
{
  return rbclipper_add_polygons_internal(self, polygons, ptClip);
}

static VALUE
rbclipper_clear(VALUE self)
{
  XCLIPPER(self)->Clear();
  return Qnil;
}

static VALUE
rbclipperoffset_clear(VALUE self)
{
  XCLIPPEROFFSET(self)->Clear();
  return Qnil;
}

static VALUE
rbclipper_multiplier(VALUE self)
{
  return rb_iv_get(self, "@multiplier");
}

static VALUE
rbclipper_multiplier_eq(VALUE self, VALUE multiplier)
{
  rb_iv_set(self, "@multiplier", multiplier);
  return multiplier;
}

static VALUE
rbclipper_orientation(VALUE self, VALUE polygonValue)
{
    double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
    ClipperLib::Path polygon;
    ary_to_polygon(polygonValue, &polygon, multiplier);

    return ClipperLib::Orientation(polygon) ? Qtrue : Qfalse;
}

static VALUE
rbclipper_area(VALUE self, VALUE polygonValue)
{
    double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
    ClipperLib::Path polygon;
    ary_to_polygon(polygonValue, &polygon, multiplier);

    return DBL2NUM(ClipperLib::Area(polygon) / multiplier / multiplier);
}

static VALUE
rbclipper_point_in_polygon(VALUE self, VALUE px, VALUE py, VALUE polygonValue)
{
    double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
    ClipperLib::Path polygon;
    ary_to_polygon(polygonValue, &polygon, multiplier);

    return INT2NUM(ClipperLib::PointInPolygon(xy_to_intpoint(px, py, multiplier), polygon));
}

static VALUE
rbclipper_offset_polygons(int argc, VALUE* argv, VALUE self)
{
    const double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
    const double inv_multiplier = 1.0 / multiplier;
    VALUE polygonsValue, deltaValue, joinTypeValue, endTypeValue;

    rb_scan_args(argc, argv, "31", &polygonsValue, &deltaValue, &joinTypeValue, &endTypeValue);

    Paths polygons;
    arys_to_polygons(polygonsValue, &polygons, multiplier);

    Paths resultPaths;
    XCLIPPEROFFSET(self)->AddPaths(polygons, sym_to_jointype(joinTypeValue), sym_to_endtype(endTypeValue));
    XCLIPPEROFFSET(self)->Execute(resultPaths, NUM2DBL(deltaValue) * multiplier);

    VALUE r = rb_ary_new();
    for(Paths::iterator i = resultPaths.begin(); i != resultPaths.end(); ++i) {
        VALUE sub = rb_ary_new();
        for(Path::iterator p = i->begin(); p != i->end(); ++p) {
            rb_ary_push(sub, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
        }
        rb_ary_push(r, sub);
    }
    return r;
}

static VALUE
rbclipper_execute_internal(VALUE self, ClipType cliptype,
                           VALUE subjfill, VALUE clipfill, VALUE resulttype)
{
  if (NIL_P(subjfill)) {
    subjfill = ID2SYM(id_even_odd);
  }

  if (NIL_P(clipfill)) {
      clipfill = ID2SYM(id_even_odd);
  }

  if (NIL_P(resulttype)) {
    resulttype = ID2SYM(id_polygons);
  }

  double inv_multiplier = 1.0 / NUM2LONG(rb_iv_get(self, "@multiplier"));

  VALUE r = rb_ary_new();
  if (resulttype == ID2SYM(id_polygons)) {
      PolyTree polytree;
      Paths solution;
      XCLIPPER(self)->Execute((ClipType) cliptype,
                              polytree,
                              sym_to_filltype(subjfill),
                              sym_to_filltype(clipfill));
      ClosedPathsFromPolyTree(polytree, solution);
      for(Paths::iterator i = solution.begin(); i != solution.end(); ++i) {
        VALUE sub = rb_ary_new();
        for(Path::iterator p = i->begin(); p != i->end(); ++p) {
          rb_ary_push(sub, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
        }
        rb_ary_push(r, sub);
      }
#ifdef use_lines
      OpenPathsFromPolyTree(polytree, solution);
      for(Paths::iterator i = solution.begin(); i != solution.end(); ++i) {
        VALUE sub = rb_ary_new();
        rb_ary_push(sub, Qnil);
        for(Path::iterator p = i->begin(); p != i->end(); ++p) {
          rb_ary_push(sub, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
        }
        rb_ary_push(r, sub);
      }
#endif
  }
  return r;
}

static VALUE
rbclipper_intersection(int argc, VALUE* argv, VALUE self)
{
  VALUE subjfill, clipfill, resulttype;
  rb_scan_args(argc, argv, "03", &subjfill, &clipfill, &resulttype);
  return rbclipper_execute_internal(self, ctIntersection, subjfill, clipfill, resulttype);
}

static VALUE
rbclipper_union(int argc, VALUE* argv, VALUE self)
{
  VALUE subjfill, clipfill, resulttype;
  rb_scan_args(argc, argv, "03", &subjfill, &clipfill, &resulttype);
  return rbclipper_execute_internal(self, ctUnion, subjfill, clipfill, resulttype);
}


static VALUE
rbclipper_difference(int argc, VALUE* argv, VALUE self)
{
  VALUE subjfill, clipfill, resulttype;
  rb_scan_args(argc, argv, "03", &subjfill, &clipfill, &resulttype);
  return rbclipper_execute_internal(self, ctDifference, subjfill, clipfill, resulttype);
}


static VALUE
rbclipper_xor(int argc, VALUE* argv, VALUE self)
{
  VALUE subjfill, clipfill, resulttype;
  rb_scan_args(argc, argv, "03", &subjfill, &clipfill, &resulttype);
  return rbclipper_execute_internal(self, ctXor, subjfill, clipfill, resulttype);
}

typedef VALUE (*ruby_method)(...);

void Init_clipper() {

    id_even_odd = rb_intern("even_odd");
    id_non_zero = rb_intern("non_zero");
    id_positive = rb_intern("positive");
    id_negative = rb_intern("negative");
    id_polygons = rb_intern("polygons");
    id_ex_polygons = rb_intern("expolygons");
    id_jtSquare = rb_intern("jtSquare");
    id_jtMiter = rb_intern("jtMiter");
    id_jtRound = rb_intern("jtRound");
    id_etClosedPolygon = rb_intern("etClosedPolygon");
    id_etClosedLine = rb_intern("etClosedLine");
    id_etOpenButt = rb_intern("etOpenButt");
    id_etOpenSquare = rb_intern("etOpenSquare");
    id_etOpenRound = rb_intern("etOpenRound");

    VALUE mod   = rb_define_module("Clipper");

    VALUE k = rb_define_class_under(mod, "Clipper", rb_cObject);
    rb_define_singleton_method(k, "new",
                             (ruby_method) rbclipper_new, 0);

    rb_define_method(k, "orientation",
                   (ruby_method) rbclipper_orientation, 1);
    rb_define_method(k, "point_in_polygon",
                   (ruby_method) rbclipper_point_in_polygon, 3);
    rb_define_method(k, "area",
                   (ruby_method) rbclipper_area, 1);
    rb_define_method(k, "add_subject_polygon",
                   (ruby_method) rbclipper_add_subject_polygon, 1);
    rb_define_method(k, "add_clip_polygon",
                   (ruby_method) rbclipper_add_clip_polygon, 1);
    rb_define_method(k, "add_subject_polyline",
                   (ruby_method) rbclipper_add_subject_polyline, 1);
    rb_define_method(k, "add_subject_polygons",
                   (ruby_method) rbclipper_add_subject_polygons, 1);
    rb_define_method(k, "add_clip_polygons",
                   (ruby_method) rbclipper_add_clip_polygons, 1);
    rb_define_method(k, "add_subject_polylines",
                   (ruby_method) rbclipper_add_subject_polylines, 1);
    rb_define_method(k, "clear!",
                   (ruby_method) rbclipper_clear, 0);
    rb_define_method(k, "intersection",
                   (ruby_method) rbclipper_intersection, -1);
    rb_define_method(k, "union",
                   (ruby_method) rbclipper_union, -1);
    rb_define_method(k, "difference",
                   (ruby_method) rbclipper_difference, -1);
    rb_define_method(k, "xor",
                   (ruby_method) rbclipper_xor, -1);

    rb_define_method(k, "multiplier",
                   (ruby_method) rbclipper_multiplier, 0);
    rb_define_method(k, "multiplier=",
                   (ruby_method) rbclipper_multiplier_eq, 1);

    rb_define_method(k, "minkowski_sum",
                   (ruby_method) rbclipper_minkowski_sum, 3);

    rb_define_method(k, "minkowski_diff",
                   (ruby_method) rbclipper_minkowski_diff, 2);

    VALUE ko = rb_define_class_under(mod, "ClipperOffset", rb_cObject);
    rb_define_singleton_method(ko, "new",
                             (ruby_method) rbclipperoffset_new, 0);
    rb_define_method(ko, "offset_polygons",
                  (ruby_method) rbclipper_offset_polygons, -1);
    rb_define_method(ko, "clear!",
                   (ruby_method) rbclipperoffset_clear, 0);

    rb_define_method(ko, "multiplier",
                  (ruby_method) rbclipper_multiplier, 0);
    rb_define_method(ko, "multiplier=",
                  (ruby_method) rbclipper_multiplier_eq, 1);
  }

} /* extern "C" */
