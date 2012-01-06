Synopsis
==========
These are Ruby bindings to Clipper, Angus Johnson's Polygon clipping
library. Because Clipper is not readily packaged, and is so beautifully
self-contained, I've included the two required files in the package.

This release contains version 4.6.3 of Clipper.

* [Clipper Homepage](http://angusj.com/delphi/clipper.php)
* [rbclipper](http://github.com/mieko/rbclipper)

To install:

    gem install clipper

Build locally:

    rake install 


Simple Usage:
===========
This shold be enough to get you started.  Full documentation is below.

    require 'clipper'

    a = [[0, 0], [0, 100], [100, 100], [100, 0]]
    b = [[-5, 50], [200, 50], [100, 5]]

    c = Clipper::Clipper.new

    c.add_subject_polygon(a)
    c.add_clip_polygon(b)
    c.union :non_zero, :non_zero

    => [[[100.0, 0.0], [0.0, 0.0], [0.0, 47.85714326530613], [-4.999999, 50.0],
         [0.0, 50.0], [0.0, 100.0], [100.0, 100.0], [100.0, 50.0],
         [200.0, 50.0], [100.0, 5.0]]]

Documentation
================

Clipper is a two-dimensional polygon clipping library.  `rbclipper`, the Ruby
bindings can be accessed by:

    require 'clipper'


Polygons
--------
Operations that accept or return polygons are specified as an array of `[x,y]` 
coordinates, for example, to specify a triangle:

    triangle = [[0,0], [0,100], [50, -100]]

Clipper supports both holes and complex polygons.  Coordinates for output 
polygons are clockwise for shells, and and counter-clockwise for holes.
See force_orientation.

Note that since 2.8, Clipper defines orientation with respect to a 
_downward-increasing Y axis_, similar to how many 2D GUI/drawing APIs position 
coordinate (0,0) at the top-left corner.  The bindings have followed Clipper 
proper in this regard.

Multiple polygons are represented as simply an array of polygons.

Fill Types
-----------
  * `:even_odd`

    A point is considered inside the polygon if the number of edge-crossings to 
    get there from outside the shape is an even number.

  * `:non_zero`

    A point is considered inside the polygon if the number of edge-crossings to 
    get there is greater than zero.

  * `:positive`

    See Clipper Homepage.

  * `:negative`

    See Clipper Homepage.



Clipper::Clipper Methods
-------

* `Clipper#initialize`

   Creates a new clipper object.

* `Clipper#add_subject_polygon(polygon)`

* `Clipper#add_clip_polygon(polygon)`

  Adds a subject or clip polygon to the engine.  Boolean operations are 
  calculated as `SUBJECT` *operatation* `CLIP`.  Multiple polygons can Pay attention 
  to the orientation of the coordinates given; counter-clockwise for shells and
  clockwise for holes.

  Multiple subject and clip polygons can be added to the engine for operations.

* `Clipper#add_subject_polygons(expolygon)`

* `Clipper#add_clip_polygons(expolygon)`

  Add an "ExPolygon" to the engine.  Which is basically a list of polygons - the first is the 
  outside (counter-clock-wise) and the rest, if any, are the holes (clock-wise).  
  Boolean operations consider every expolygon added in this manner to be the same object.

* `Clipper#multiplier`

* `Clipper#multiplier=`

  Defaults to 2^10 = 1048576. Clipper since version 4.0 uses integer math instead of floating point. 
  To simplify using floating point coordinates, this multiplier is multiplied to each coordinate value 
  before beeing sent to Clipper, and each result coordinate is divided by the multiplier. Use 1 if you
  want to use integer coordinates.

* `Clipper#intersection(subject_fill=:even_odd, clip_fill=:even_odd, result_type=:polygons)`

* `Clipper#union(subject_fill=:even_odd, clip_fill=:even_odd, result_type=:polygons)`

* `Clipper#difference(subject_fill=:even_odd, clip_fill=:even_odd, result_type=:polygons)`

* `Clipper#xor(subject_fill=:even_odd, clip_fill=:even_odd, result_type=:polygons)`

   Performs a boolean operation on the polygons that have been added to the 
   clipper object.  The result is a list of polygons or expolygons, depending on result_type being :polygons or :expolygons

* `Clipper#offset_polygons(polygons, delta, join_type, miter_limit=0)`

  Expands the polygons by delta. Use negative delta to compress the polygons. join_type is any of :jtSquare, :jtButt, :jtMiter or :jtRound. 
  Use miter_limit to make sharp joints not extend too long, by cutting off the edge. 

* `Clipper#area(polygon)`

  Returns the area of the supplied polygon. The returned area is negative if the polygon points are oriented clockwise, positive otherwise. Obeys the multiplier and use_full_coordinate_range settings for the clipper object.

* `Clipper#Orientation(polygon)`

  Orientation returns a boolean value that is based on the polygon's orientation relative to the display's orientation (ie Y-axis positive upward vs Y-axis positive downward). 

* On Y-axis positive upward displays, Orientation will return true if the polygon's orientation is counter-clockwise. 
* On Y-axis positive downward displays, Orientation will return true if the polygon's orientation is clockwise. 


