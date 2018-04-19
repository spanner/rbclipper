# frozen_string_literal: true
require 'clipper'

require 'simplecov'
require 'simplecov-json'
# require 'stackprof'

SimpleCov.formatter = SimpleCov::Formatter::JSONFormatter
SimpleCov.start

require 'minitest/autorun'
require 'minitest/pride'


# Helper class to mange rectangles
class Rectangle
  def initialize(pos, size)
    @r = [[Float(pos[0]), Float(pos[1])], [Float(size[0]), Float(size[1])]]
  end

  def pos
    @r[0]
  end

  def pos=(val)
    @r[0] = val
  end

  def size
    @r[1]
  end

  def size=(val)
    @r[1] = val
  end

  def height
    @r[1][1]
  end

  def <=>(other)
    height * width <=> other.height * other.width
  end

  def width
    @r[1][0]
  end

  def translate_by!(delta)
    self.pos += delta
    self
  end

  def translate_by(delta)
    Rectangle.new(self.pos + delta, size)
  end

  def to_poly
    [[@r[0][0], @r[0][1]], [@r[0][0] + @r[1][0], @r[0][1]], [@r[0][0] + @r[1][0], @r[0][1] + @r[1][1]], [@r[0][0], @r[0][1] + @r[1][1]]]
  end

  def to_a
    to_poly
  end
end
