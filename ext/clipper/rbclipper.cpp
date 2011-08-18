/* 
 * Clipper Ruby Bindings
 * Copyright 2010 Mike Owens <http://mike.filespanker.com/>
 * Changed by Dag Rende for Clipper 4.4.0
 *
 * Released under the same terms as Clipper.
 *
 */

#include <clipper.hpp>
#include <ruby.h>

#ifndef DBL2NUM
# define DBL2NUM rb_float_new
#endif

using namespace std;
using namespace clipper;

static ID id_even_odd;
static ID id_non_zero;
static ID id_polygons;
static ID id_expolygons;

static inline Clipper*
XCLIPPER(VALUE x)
{
  Clipper* clipper;
  Data_Get_Struct(x, Clipper, clipper);
  return clipper;
}

static inline PolyFillType
sym_to_filltype(VALUE sym)
{
  ID inp = rb_to_id(sym);

  if (inp == id_even_odd) {
    return pftEvenOdd;
  } else if (inp == id_non_zero) {
    return pftNonZero;
  }

  rb_raise(rb_eArgError, "%s", "Expected :even_odd or :non_zero");
}

extern "C" {

static void
ary_to_polygon(VALUE ary, clipper::Polygon* poly, double multiplier)
{
  const char* earg =
    "Polygons have format: [[p0_x, p0_y], [p1_x, p1_y], ...]";

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
rbclipper_free(void* ptr)
{
  delete (Clipper*) ptr;
}

static VALUE
rbclipper_new(VALUE klass)
{
  Clipper* ptr = new Clipper;
  VALUE r = Data_Wrap_Struct(klass, 0, rbclipper_free, ptr);
  rb_obj_call_init(r, 0, 0);
  rb_iv_set(r, "@multiplier", INT2NUM(1000));
  return r;
}

static VALUE
rbclipper_add_polygon_internal(VALUE self, VALUE polygon,
                               PolyType polytype)
{
  clipper::Polygon tmp;
  double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
  ary_to_polygon(polygon, &tmp, multiplier);
  XCLIPPER(self)->AddPolygon(tmp, polytype);
  return Qnil;
}

static VALUE
rbclipper_add_polygons_internal(VALUE self, VALUE polygonsValue, PolyType polytype) {
  double multiplier = NUM2DBL(rb_iv_get(self, "@multiplier"));
  Polygons polygons;
  for(long i = 0; i != RARRAY_LEN(polygonsValue); i++) {
    VALUE sub = rb_ary_entry(polygonsValue, i);
    Check_Type(sub, T_ARRAY);

    clipper::Polygon tmp;
    ary_to_polygon(sub, &tmp, multiplier);
    polygons.push_back(tmp);
  }
  XCLIPPER(self)->AddPolygons(polygons, polytype);
  return Qnil;
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
rbclipper_use_full_coordinate_range(VALUE self)
{
  return XCLIPPER(self)->UseFullCoordinateRange() ? Qtrue : Qfalse;
}

static VALUE
rbclipper_use_full_coordinate_range_eq(VALUE self, VALUE b)
{
  XCLIPPER(self)->UseFullCoordinateRange(b == Qtrue);
  return b;
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
      Polygons solution;
      XCLIPPER(self)->Execute((ClipType) cliptype,
                              solution,
                              sym_to_filltype(subjfill),
                              sym_to_filltype(clipfill));
      for(Polygons::iterator i = solution.begin(); i != solution.end(); ++i) {
        VALUE sub = rb_ary_new();
        for(Polygon::iterator p = i->begin(); p != i->end(); ++p) {
          rb_ary_push(sub, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
        }
        rb_ary_push(r, sub);
      }
  } else {
      ExPolygons solution;
      XCLIPPER(self)->Execute((ClipType) cliptype,
                              solution,
                              sym_to_filltype(subjfill),
                              sym_to_filltype(clipfill));
      for(ExPolygons::iterator i = solution.begin(); i != solution.end(); ++i) {
        VALUE expolygon_arr = rb_ary_new();

        VALUE outer_arr = rb_ary_new();
        for(Polygon::iterator p = i->outer.begin(); p != i->outer.end(); ++p) {
          rb_ary_push(outer_arr, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
        }
        rb_ary_push(expolygon_arr, outer_arr);

        for(Polygons::iterator ps = i->holes.begin(); ps != i->holes.end(); ++ps) {
          VALUE hole_arr = rb_ary_new();
          for(Polygon::iterator p = ps->begin(); p != ps->end(); ++p) {
            rb_ary_push(hole_arr, rb_ary_new3(2, DBL2NUM(p->X * inv_multiplier), DBL2NUM(p->Y * inv_multiplier)));
          }
          rb_ary_push(expolygon_arr, hole_arr);
        }

        rb_ary_push(r, expolygon_arr);
      }
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
  id_polygons = rb_intern("polygons");
  id_expolygons = rb_intern("expolygons");

  VALUE mod   = rb_define_module("Clipper");

  VALUE k = rb_define_class_under(mod, "Clipper", rb_cObject);
  rb_define_singleton_method(k, "new",
                             (ruby_method) rbclipper_new, 0);

  rb_define_method(k, "add_subject_polygon",
                   (ruby_method) rbclipper_add_subject_polygon, 1);
  rb_define_method(k, "add_clip_polygon",
                   (ruby_method) rbclipper_add_clip_polygon, 1);
  rb_define_method(k, "add_subject_polygons",
                   (ruby_method) rbclipper_add_subject_polygons, 1);
  rb_define_method(k, "add_clip_polygons",
                   (ruby_method) rbclipper_add_clip_polygons, 1);
  rb_define_method(k, "clear!",
                   (ruby_method) rbclipper_clear, 0);
  rb_define_method(k, "use_full_coordinate_range",
                   (ruby_method) rbclipper_use_full_coordinate_range, 0);
  rb_define_method(k, "use_full_coordinate_range=",
                   (ruby_method) rbclipper_use_full_coordinate_range_eq, 1);
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
}

} /* extern "C" */
