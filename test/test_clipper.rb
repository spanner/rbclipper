require 'test_helper'

class ClipperTest < Minitest::Test

  def setup
    @clip = Clipper::Clipper.new
  end

  def test_area
    r = Rectangle.new([0,0], [10, -10])
    assert_equal(@clip.area(r.to_poly), r.width * r.height)

    r = Rectangle.new([0,0], [5, 5])
    assert_equal(@clip.area(r.to_poly), r.width * r.height)
  end

  def test_orientation
    r = Rectangle.new([0,0], [10, 10])
    assert(@clip.orientation(r.to_poly))

    r = Rectangle.new([0,0], [10, -10])
    assert(not(@clip.orientation(r.to_poly)))
  end

  def test_point_in_polygon
    poly = Rectangle.new([0,0], [1.12345678, 10]).to_poly

    # point in polygon
    assert_equal(1, @clip.point_in_polygon(0.1, 0.1, poly))

    # point outside polygon
    assert_equal(0, @clip.point_in_polygon(-0.1, 0.1, poly))

    # point on polygon
    assert_equal(-1, @clip.point_in_polygon(0, 0, poly))

    # poiny on polygon 6 decimals
    assert_equal(-1, @clip.point_in_polygon(1.1234567, 5, poly))
    assert_equal(0, @clip.point_in_polygon(1.123457, 5, poly))
    assert_equal(1, @clip.point_in_polygon(1.123455, 5, poly))
  end

  def test_clip
    p1 = Rectangle.new([0,0], [2, 2]).to_poly
    p2 = Rectangle.new([0,0], [1, 1]).to_poly

    @clip.add_subject_polygon(p1)
    @clip.add_clip_polygon(p2)

    p3 = @clip.difference(:non_zero, :non_zero)

    assert_equal(1, p3.size)
    assert @clip.orientation(p3[0])
    assert_equal(3, @clip.area(p3[0]))
  end

  # FIXME: Add more tests for polylines, as this feature was disabled by default,
  # and we neglected to implement support for polytrees.
  def test_intersection_polyline
    p1 = Rectangle.new([0,0], [2, 2]).to_poly
    line = [[-1,1], [3,1]]

    @clip.add_subject_polyline(line)
    @clip.add_clip_polygon(p1)

    p3 = @clip.intersection(:non_zero, :non_zero)

    assert_equal 1, p3.size

    # Polyline is identified by nil as the first element (workaround for not handling polytrees)
    assert_nil p3[0][0]
    assert_equal [2,1], p3[0][1]
    assert_equal [0,1], p3[0][2]
  end

  def test_union
    p1 = Rectangle.new([0,0], [1, 1]).to_poly
    p2 = Rectangle.new([1,0], [1, 1]).to_poly

    @clip.add_subject_polygon(p1)
    @clip.add_subject_polygon(p2)

    p3 = @clip.union(:non_zero, :non_zero)

    assert_equal(1, p3.size)
    assert @clip.orientation(p3[0])
    assert_equal(2, @clip.area(p3[0]))
  end

  def test_minkowski_sum
    p1 = Rectangle.new([0,0], [1, 1]).to_poly
    p2 = Rectangle.new([0,0], [1, 1]).to_poly

    p3 = @clip.minkowski_sum(p1, [p2], true)

    assert_equal(1, p3.size)
    assert @clip.orientation(p3[0])
    assert_equal(4, @clip.area(p3[0]))
  end

  #FIXME: Tests go not as deep into the edgecases as they should, especially for non convex polygons
  def test_minkowski_diff
    p1 = Rectangle.new([0,0], [1, 1]).to_poly
    p2 = Rectangle.new([1,1], [0.5, 0.5]).to_poly

    p3 = @clip.minkowski_diff(p1, p2)

    assert_equal(2, p3.size)
    assert @clip.orientation(p3[0])
    assert not(@clip.orientation(p3[1]))
    assert_equal(2.25, @clip.area(p3[0]))
    assert_equal(-0.25, @clip.area(p3[1]))
  end
end
