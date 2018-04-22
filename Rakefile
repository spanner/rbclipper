require 'bundler'
Bundler::GemHelper.install_tasks

# require 'rake/extensiontask'

# Rake::ExtensionTask.new('clipper')

require 'rake/testtask'

Rake::TestTask.new do |t|
  t.libs << 'test'
  t.pattern = "test/test_*.rb"
  t.verbose = false
  t.options = "--max-duration=0.1 --junit --verbose"
end
