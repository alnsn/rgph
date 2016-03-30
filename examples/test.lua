local rgph = require "rgph"

local args = { ... }
local seed = tonumber(args[1] or "0")

local keys = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10,
               k=11, l=12, m=13, n=14, o=15, p=16, q=17, r=18,
	       s=19, t=20, u=21, v=22, w=23, x=24, y=25, z=26 }

local function assign(keys, seed, flags)
	local ntries = 20
	local bad_seeds = {}
	local nkeys = rgph.count_keys(pairs(keys))
	local graph = rgph.new_graph(nkeys, flags)

	for i = 1, ntries do
		local built, err = graph:build(seed, pairs(keys))
		if built then
			return assert(graph:assign())
		end
		assert(not err, err) -- retry if cyclic, throw on other errors
		bad_seeds[i] = seed
		seed = seed + 1
	end

	-- The keys table can't have duplicate keys, this loop
	-- is only required to handle a general case.
	for i = ntries, 1, -1 do
		local dup, a, b = graph:find_duplicates(pairs(keys))
		if dup then
			return nil, a, b
		elseif a == "try again" and i > 1 then
			local built, err =
			    graph:build(bad_seeds[i - 1], pairs(keys))
			assert(not built and not err, err)
		else
			error(a)
		end
	end
end

print(assign(keys, seed, "jenkins2v"))
print(assign(keys, seed, "murmur32v,rank3")) -- Murmur32 isn't good enough
