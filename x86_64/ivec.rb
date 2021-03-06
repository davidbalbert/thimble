#!/usr/bin/env ruby

puts "# generated by ivec.rb - do not edit"
puts ".globl alltraps"

256.times do |i|
  puts ".globl vector#{i}"
  puts "vector#{i}:"

  unless i == 8 || (10..14).include?(i) || i == 17
    puts "    pushq $0       # Error code"
  end

  puts "    pushq $#{i}"
  puts "    jmp alltraps"
end

puts
puts ".data"
puts ".globl vectors"
puts "vectors:"

256.times do |i|
  puts "    .quad vector#{i}"
end
