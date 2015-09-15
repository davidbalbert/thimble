#!/usr/bin/env ruby

def die(s)
  $stderr.puts "#{$0}: #{s}"
  exit 1
end

s = nil

File.open("mbr", "rb") do |f|
  s = f.read
end

if s.size > 510
  die "mbr is too big (510 max)"
end

$stderr.puts "mbr size is #{s.size}"

s += "\x00".b * (510 - s.size)
s += "\x55\xAA".b

File.open("mbr", "wb") do |f|
  f.write(s)
end
