# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require File.expand_path("../lib/clipper/version", __FILE__)

Gem::Specification.new do |s|
  s.name        = "rbclipper"
  s.version     = Clipper::VERSION
  s.platform    = Gem::Platform::RUBY
  s.authors     = ['Angus Johnson','Mike Owens', 'Dag Rende', 'Jaap van der Plas', 'Simon George', 'Kjell Morgenstern', 'Will Ross']
  s.email       = ['will@spanner.org']
  s.homepage    = "http://github.com/spanner/rbclipper"
  s.summary     = "Ruby wrapper for Clipper, Angus Johnson's excellent polygon clipping library"
  s.description = "Builds a native ruby extension for Clipper"
  s.licenses     = "Boost Software License - Version 1.0"

  s.extra_rdoc_files  = %w(README.md Changelog ext/clipper/rbclipper.cpp)
  s.rdoc_options.concat %w(--main=README.md)

  s.required_rubygems_version = ">= 1.3.6"
  s.rubyforge_project         = "clipper"

  s.add_development_dependency "bundler", ">= 1.5"
  s.add_development_dependency "rake"
  s.add_development_dependency "minitest", ">= 5.8"
  s.add_development_dependency "minitest-junit"
  s.add_development_dependency "minitest-snail"
  # s.add_development_dependency "minitest-reporters", ">= 1.1"

  s.files        = Dir["**/clipper/*", "LICENSE*", "Changelog", "Rakefile", "Gemfile", "*.md"]
  s.executables  = Dir["**/clipper/*.so"]
  s.require_path = 'lib'

  s.extensions = "ext/clipper/extconf.rb"
end
