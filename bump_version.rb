#!/usr/bin/env ruby

require 'find'
require 'json'
require 'semantic'
require 'git'

# command line options

level = :patch
unless ARGV[0].nil?
	level = :minor if ARGV[0] == "minor"
	level = :major if ARGV[0] == "major"
end

# get current version. Use version.json as definitive source
file = File.read('version.json')

version = Semantic::Version.new JSON.parse(file)['version']

# for now, just bump patch. Add other options to CLI interface later
new_version = version.increment!(level)

puts "#{version} => #{new_version}"

# checkout a new local testing branch
g = Git.init
g.branch("testing/#{new_version}").checkout

# update all the files
Find.find('./') do |path|
    next unless File.file?(path)
    next if /\.git/.match File.dirname(path)
    next if /\.idea/.match File.dirname(path)
    `sed -i '' 's/#{version}/#{new_version}/g' "#{path}"`
end

# update the changelog
changelog = "---\nlayout: changelog\n---\n\n" + File.read("docs/_posts/recent_changes.md")
changelogfilename = "docs/_posts/" + Time.now.strftime("%Y-%m-%d") + "-" + new_version.to_s() + ".md"

File.write(changelogfilename, changelog)
File.write("docs/_posts/recent_changes.md", "- put changelogs here")


g.commit_all("Bump version to #{new_version}")
