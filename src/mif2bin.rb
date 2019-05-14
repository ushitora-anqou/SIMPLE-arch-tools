#!/usr/bin/ruby

require "byebug"

raise "Usage: ruby mif2bin.rb input-mif-file-path output-bin-file-path" unless ARGV.size == 2

content_begin = nil
content_end = nil
address_radix = nil
data_radix = nil
open(ARGV[0]).each_line.with_index do |line, index|
  content_begin = index + 1 if line =~ /^CONTENT BEGIN/
  content_end = index - 1 if line =~ /^END/
  address_radix = 16 if line =~ /^ADDRESS_RADIX=HEX/
  data_radix = 10 if line =~ /^DATA_RADIX=DEC/
end
raise "Some wrong with this MIF" if content_begin.nil?
raise "Some wrong with this MIF" if content_end.nil?
raise "Some wrong with this MIF" if address_radix.nil?
raise "Some wrong with this MIF" if data_radix.nil?

data = []
open(ARGV[0]).read.split("\n")[content_begin..content_end].each do |line|
  # e.g. [000..400] : 0;
  if line =~ /
              ^\s*
              \[\s*
              ([0-9a-f]+)\s*
              \.\.\s*
              ([0-9a-f]+)\s*
              \]\s*
              :\s*
              (-?[0-9a-f]+)\s*
              ;\s*
              $
             /xi
    range = ($1.to_i(address_radix)..$2.to_i(address_radix))
    value = $3.to_i(data_radix)
    range.each do |i|
      data[i] = value
    end
  end

  # e.g. 401  :  8620;
  if line =~ /
              ^\s*
              ([0-9a-f]+)\s*
              :\s*
              (-?[0-9a-f]+)\s*
              ;\s*
              $
             /xi
    data[$1.to_i(address_radix)] = $2.to_i(data_radix)
  end
end

open(ARGV[1], "w") do |fh|
  fh.write data.pack("n*")
end
