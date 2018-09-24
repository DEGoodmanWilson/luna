#!/usr/bin/env ruby

require 'find'
require 'json'
require 'semantic'
require 'git'

# get current version. Use version.json as definitive source
file = File.read('version.json')

version = Semantic::Version.new JSON.parse(file)['version']

# for now, just bump patch. Add other options to CLI interface later
new_version = version.increment!(:patch)

puts "#{version} => #{new_version}"

# checkout a new local testing branch
g = Git.init
g.branch("testing/#{new_version}").checkout

# update all the files
Find.find('./') do |path|
    `sed -i '' 's/#{version}/#{new_version}/g' #{path}`
end
