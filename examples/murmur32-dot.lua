-- 32bit Murmur 3 hangs on keys a-z when trying to find a good 3-graph.
-- This script draws generated 3-graphs in graphviz dot format.
-- Usage:
--     lua murmur32-dot.lua $RANDOM | dot -Tpng -o /path/to/out.png.
local rgph = require "rgph"
local rgphdot = require "rgphdot"

local args = { ... }
local seed = tonumber(args[1] or "123456789")

local keys = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10,
               k=11, l=12, m=13, n=14, o=15, p=16, q=17, r=18,
               s=19, t=20, u=21, v=22, w=23, x=24, y=25, z=26 }

local nkeys = rgph.count_keys(pairs(keys))
local g = rgph.new_graph(nkeys, "murmur32v,rank3")
local ok, err = g:build(seed, pairs(keys))
if not ok and err then error(err) end

rgphdot.dot(io.stdout, g, pairs(keys))
