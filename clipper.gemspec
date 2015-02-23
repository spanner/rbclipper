# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'vector_salad/version'

Gem::Specification.new do |s|
  s.name        = "ruby_clipper"
  s.version     = Clipper::VERSION
  s.platform    = Gem::Platform::RUBY
  s.authors     = ['Angus Johnson','Mike Owens', 'Dag Rende', 'Jaap van der Plas', 'Simon George']
  s.email       = ['simon@sfcgeorge.co.uk']
  s.homepage    = "http://github.com/sfcgeorge/rbclipper"
  s.summary     = "Ruby wrapper for Clipper, Angus Johnson's excellent polygon clipping library"
  s.description = "Builds a native ruby extension for Clipper"
  s.licence     = "Boost Software License - Version 1.0"

  s.extra_rdoc_files  = %w(README.md Changelog ext/clipper/rbclipper.cpp)
  s.rdoc_options.concat %w(--main=README.md)

  s.required_rubygems_version = ">= 1.3.6"

  s.add_development_dependency "bundler", ">= 1.0.0"
  s.files        = Dir["**/clipper/*", "LICENSE*", "Rakefile", "Gemfile", "Changelog", "*.md"]
  # s.executables  = Dir["**/clipper/*.so"]
  s.require_path = 'lib'

  s.extensions = "ext/clipper/extconf.rb"
end
