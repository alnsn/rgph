local rgph = require "rgph"

local args = { ... }
local nkeys = tonumber(args[1] or "1")
local seed  = tonumber(args[2] or "0")

local f = io.open("/usr/share/dict/words")
local g = rgph.new_graph(nkeys, "jenkins2")
print(g:entries(), g:vertices())
print(g:build(seed, f:lines()))
