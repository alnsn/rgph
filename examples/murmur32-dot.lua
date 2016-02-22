local rgph = require "rgph"
local rgphdot = require "rgphdot"

local args = { ... }
local nkeys = tonumber(args[1] or "1")
local seed  = tonumber(args[2] or "0")

local f = io.open("/usr/share/dict/words")
local g = rgph.new_graph(nkeys, "jenkins2,rank2")
local ok, err = g:build(seed, f:lines())
if err then error(err) end

rgphdot(io.stdout, g, io.open("/usr/share/dict/words"):lines())
