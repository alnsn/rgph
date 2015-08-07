local rgph = require "rgph"

local args = { ... }
local nkeys = tonumber(args[1] or "8")
local seed  = tonumber(args[2] or "0")

local keys = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8 }
local g = rgph.new_graph(nkeys, "jenkins2")
print(g:entries(), g:vertices())

local ok,err = g:build(seed, pairs(keys))
print(ok, err)

if not ok and not err then
	print("core", g:core_size())
end

for k,h1,h2,h3 in g:edges(pairs(keys)) do
	print(k,h1,h2,h3)
end
