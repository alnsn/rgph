local rgph = require "rgph"

local args = { ... }
local nkeys = tonumber(args[1] or "1")
local seed  = tonumber(args[2] or "0")
local file  = args[3] or "/usr/share/dict/words"

local f = io.open(file)
local g = rgph.new_graph(nkeys, "jenkins2v,rank3,chm")
print(g:entries(), g:vertices())

local ok,err = g:build(seed, f:lines())
f:close()
print(ok, err)

if not ok and not err then
	local f = io.open(file)
	print("duplicates", g:find_duplicates(f:lines()))
	f:close()
end

if not err then
	print(g:edge(1))
	print(g:edge(nkeys))
end

local assign = assert(g:assign())
print(assign[1], assign[g:entries()])
