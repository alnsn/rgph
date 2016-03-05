local rgph = require "rgph"

local args = { ... }
local seed = tonumber(args[1] or "123456789")

local keys = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10,
               k=11, l=12, m=13, n=14, o=15, p=16, q=17, r=18,
               s=19, t=20, u=21, v=22, w=23, x=24, y=25, z=26 }

local function test(keys, seed, flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)
	local rank = g:rank()
	local nverts = g:vertices()

	-- XXX pow2 and fastdiv checks

	while true do
		local ok, err = g:build(seed, pairs(keys))
		if ok then break end
		assert(not err, err) -- pairs(keys) can't fail
		seed = seed + 1
	end

	-- Numbers are converted to strings by Lua
	assert(g:datalen_min() == 1)
	assert(g:datalen_max() == 2)

	local assign = assert(g:assign())

	local bdz = g:algo() == "bdz"
	local unassigned = bdz and rank or nkeys
	for v = 0, nverts - 1 do
		assert(assign[v] >= 0 and assign[v] <= unassigned)
	end

	if bdz then
		local duplicates = {}
		for e = 1, nkeys do
			local edges = { g:edge(e) }

			local i = 1
			for j = 1, rank do
				local v = edges[j]
				local a = assign[v]
				assert(a < unassigned, "must be assigned")
				i = i + a
				if i > unassigned then i = i - unassigned end
			end

			local h = edges[i]
			assert(not duplicates[h], "hashes must be unique")
			duplicates[h] = true
		end
	else
		for e = 1, nkeys do
			local edges = { g:edge(e) }

			local h = 1
			for j = 1, rank do
				local v = edges[j]
				local a = assign[v]
				assert(a < unassigned, "must be assigned")
				h = h + a
				if h > unassigned then h = h - unassigned end
			end

			assert(h == e, "must be order-preserving minimal hash")
		end
	end
end

test(keys, seed)

test(keys, seed, "jenkins2")
test(keys, seed, "jenkins2,chm")
test(keys, seed, "jenkins2,bdz")
test(keys, seed, "jenkins2,pow2")
test(keys, seed, "jenkins2,pow2,chm")
test(keys, seed, "jenkins2,pow2,bdz")
test(keys, seed, "jenkins2,fastdiv")
test(keys, seed, "jenkins2,fastdiv,chm")
test(keys, seed, "jenkins2,fastdiv,bdz")
test(keys, seed, "jenkins2,pow2,fastdiv")
test(keys, seed, "jenkins2,pow2,fastdiv,chm")
test(keys, seed, "jenkins2,pow2,fastdiv,bdz")
test(keys, seed, "jenkins2,rank2")
test(keys, seed, "jenkins2,chm,rank2")
test(keys, seed, "jenkins2,bdz,rank2")
test(keys, seed, "jenkins2,pow2,rank2")
test(keys, seed, "jenkins2,pow2,chm,rank2")
test(keys, seed, "jenkins2,pow2,bdz,rank2")
test(keys, seed, "jenkins2,fastdiv,rank2")
test(keys, seed, "jenkins2,fastdiv,chm,rank2")
test(keys, seed, "jenkins2,fastdiv,bdz,rank2")
test(keys, seed, "jenkins2,pow2,fastdiv,rank2")
test(keys, seed, "jenkins2,pow2,fastdiv,chm,rank2")
test(keys, seed, "jenkins2,pow2,fastdiv,bdz,rank2")
test(keys, seed, "jenkins2,rank3")
test(keys, seed, "jenkins2,chm,rank3")
test(keys, seed, "jenkins2,bdz,rank3")
test(keys, seed, "jenkins2,pow2,rank3")
test(keys, seed, "jenkins2,pow2,chm,rank3")
test(keys, seed, "jenkins2,pow2,bdz,rank3")
test(keys, seed, "jenkins2,fastdiv,rank3")
test(keys, seed, "jenkins2,fastdiv,chm,rank3")
test(keys, seed, "jenkins2,fastdiv,bdz,rank3")
test(keys, seed, "jenkins2,pow2,fastdiv,rank3")
test(keys, seed, "jenkins2,pow2,fastdiv,chm,rank3")
test(keys, seed, "jenkins2,pow2,fastdiv,bdz,rank3")

-- XXX "murmur32,rank3" tests hang
--test(keys, seed, "murmur32")
--test(keys, seed, "murmur32,chm")
--test(keys, seed, "murmur32,bdz")
test(keys, seed, "murmur32,pow2")
test(keys, seed, "murmur32,pow2,chm")
test(keys, seed, "murmur32,pow2,bdz")
--test(keys, seed, "murmur32,fastdiv")
--test(keys, seed, "murmur32,fastdiv,chm")
--test(keys, seed, "murmur32,fastdiv,bdz")
--test(keys, seed, "murmur32,pow2,fastdiv")
--test(keys, seed, "murmur32,pow2,fastdiv,chm")
--test(keys, seed, "murmur32,pow2,fastdiv,bdz")
test(keys, seed, "murmur32,rank2")
test(keys, seed, "murmur32,chm,rank2")
test(keys, seed, "murmur32,bdz,rank2")
test(keys, seed, "murmur32,pow2,rank2")
test(keys, seed, "murmur32,pow2,chm,rank2")
test(keys, seed, "murmur32,pow2,bdz,rank2")
test(keys, seed, "murmur32,fastdiv,rank2")
test(keys, seed, "murmur32,fastdiv,chm,rank2")
test(keys, seed, "murmur32,fastdiv,bdz,rank2")
test(keys, seed, "murmur32,pow2,fastdiv,rank2")
test(keys, seed, "murmur32,pow2,fastdiv,chm,rank2")
test(keys, seed, "murmur32,pow2,fastdiv,bdz,rank2")
--test(keys, seed, "murmur32,rank3")
--test(keys, seed, "murmur32,chm,rank3")
--test(keys, seed, "murmur32,bdz,rank3")
test(keys, seed, "murmur32,pow2,rank3")
test(keys, seed, "murmur32,pow2,chm,rank3")
test(keys, seed, "murmur32,pow2,bdz,rank3")
--test(keys, seed, "murmur32,fastdiv,rank3")
--test(keys, seed, "murmur32,fastdiv,chm,rank3")
--test(keys, seed, "murmur32,fastdiv,bdz,rank3")
--test(keys, seed, "murmur32,pow2,fastdiv,rank3")
--test(keys, seed, "murmur32,pow2,fastdiv,chm,rank3")
--test(keys, seed, "murmur32,pow2,fastdiv,bdz,rank3")

test(keys, seed, "murmur32s")
test(keys, seed, "murmur32s,chm")
test(keys, seed, "murmur32s,bdz")
test(keys, seed, "murmur32s,pow2")
test(keys, seed, "murmur32s,pow2,chm")
test(keys, seed, "murmur32s,pow2,bdz")
test(keys, seed, "murmur32s,fastdiv")
test(keys, seed, "murmur32s,fastdiv,chm")
test(keys, seed, "murmur32s,fastdiv,bdz")
test(keys, seed, "murmur32s,pow2,fastdiv")
test(keys, seed, "murmur32s,pow2,fastdiv,chm")
test(keys, seed, "murmur32s,pow2,fastdiv,bdz")
test(keys, seed, "murmur32s,rank2")
test(keys, seed, "murmur32s,chm,rank2")
test(keys, seed, "murmur32s,bdz,rank2")
test(keys, seed, "murmur32s,pow2,rank2")
test(keys, seed, "murmur32s,pow2,chm,rank2")
test(keys, seed, "murmur32s,pow2,bdz,rank2")
test(keys, seed, "murmur32s,fastdiv,rank2")
test(keys, seed, "murmur32s,fastdiv,chm,rank2")
test(keys, seed, "murmur32s,fastdiv,bdz,rank2")
test(keys, seed, "murmur32s,pow2,fastdiv,rank2")
test(keys, seed, "murmur32s,pow2,fastdiv,chm,rank2")
test(keys, seed, "murmur32s,pow2,fastdiv,bdz,rank2")
test(keys, seed, "murmur32s,rank3")
test(keys, seed, "murmur32s,chm,rank3")
test(keys, seed, "murmur32s,bdz,rank3")
test(keys, seed, "murmur32s,pow2,rank3")
test(keys, seed, "murmur32s,pow2,chm,rank3")
test(keys, seed, "murmur32s,pow2,bdz,rank3")
test(keys, seed, "murmur32s,fastdiv,rank3")
test(keys, seed, "murmur32s,fastdiv,chm,rank3")
test(keys, seed, "murmur32s,fastdiv,bdz,rank3")
test(keys, seed, "murmur32s,pow2,fastdiv,rank3")
test(keys, seed, "murmur32s,pow2,fastdiv,chm,rank3")
test(keys, seed, "murmur32s,pow2,fastdiv,bdz,rank3")

test(keys, seed, "xxh32s")
test(keys, seed, "xxh32s,chm")
test(keys, seed, "xxh32s,bdz")
test(keys, seed, "xxh32s,pow2")
test(keys, seed, "xxh32s,pow2,chm")
test(keys, seed, "xxh32s,pow2,bdz")
test(keys, seed, "xxh32s,fastdiv")
test(keys, seed, "xxh32s,fastdiv,chm")
test(keys, seed, "xxh32s,fastdiv,bdz")
test(keys, seed, "xxh32s,pow2,fastdiv")
test(keys, seed, "xxh32s,pow2,fastdiv,chm")
test(keys, seed, "xxh32s,pow2,fastdiv,bdz")
test(keys, seed, "xxh32s,rank2")
test(keys, seed, "xxh32s,chm,rank2")
test(keys, seed, "xxh32s,bdz,rank2")
test(keys, seed, "xxh32s,pow2,rank2")
test(keys, seed, "xxh32s,pow2,chm,rank2")
test(keys, seed, "xxh32s,pow2,bdz,rank2")
test(keys, seed, "xxh32s,fastdiv,rank2")
test(keys, seed, "xxh32s,fastdiv,chm,rank2")
test(keys, seed, "xxh32s,fastdiv,bdz,rank2")
test(keys, seed, "xxh32s,pow2,fastdiv,rank2")
test(keys, seed, "xxh32s,pow2,fastdiv,chm,rank2")
test(keys, seed, "xxh32s,pow2,fastdiv,bdz,rank2")
test(keys, seed, "xxh32s,rank3")
test(keys, seed, "xxh32s,chm,rank3")
test(keys, seed, "xxh32s,bdz,rank3")
test(keys, seed, "xxh32s,pow2,rank3")
test(keys, seed, "xxh32s,pow2,chm,rank3")
test(keys, seed, "xxh32s,pow2,bdz,rank3")
test(keys, seed, "xxh32s,fastdiv,rank3")
test(keys, seed, "xxh32s,fastdiv,chm,rank3")
test(keys, seed, "xxh32s,fastdiv,bdz,rank3")
test(keys, seed, "xxh32s,pow2,fastdiv,rank3")
test(keys, seed, "xxh32s,pow2,fastdiv,chm,rank3")
test(keys, seed, "xxh32s,pow2,fastdiv,bdz,rank3")

test(keys, seed, "xxh64s")
test(keys, seed, "xxh64s,chm")
test(keys, seed, "xxh64s,bdz")
test(keys, seed, "xxh64s,pow2")
test(keys, seed, "xxh64s,pow2,chm")
test(keys, seed, "xxh64s,pow2,bdz")
test(keys, seed, "xxh64s,fastdiv")
test(keys, seed, "xxh64s,fastdiv,chm")
test(keys, seed, "xxh64s,fastdiv,bdz")
test(keys, seed, "xxh64s,pow2,fastdiv")
test(keys, seed, "xxh64s,pow2,fastdiv,chm")
test(keys, seed, "xxh64s,pow2,fastdiv,bdz")
test(keys, seed, "xxh64s,rank2")
test(keys, seed, "xxh64s,chm,rank2")
test(keys, seed, "xxh64s,bdz,rank2")
test(keys, seed, "xxh64s,pow2,rank2")
test(keys, seed, "xxh64s,pow2,chm,rank2")
test(keys, seed, "xxh64s,pow2,bdz,rank2")
test(keys, seed, "xxh64s,fastdiv,rank2")
test(keys, seed, "xxh64s,fastdiv,chm,rank2")
test(keys, seed, "xxh64s,fastdiv,bdz,rank2")
test(keys, seed, "xxh64s,pow2,fastdiv,rank2")
test(keys, seed, "xxh64s,pow2,fastdiv,chm,rank2")
test(keys, seed, "xxh64s,pow2,fastdiv,bdz,rank2")
test(keys, seed, "xxh64s,rank3")
test(keys, seed, "xxh64s,chm,rank3")
test(keys, seed, "xxh64s,bdz,rank3")
test(keys, seed, "xxh64s,pow2,rank3")
test(keys, seed, "xxh64s,pow2,chm,rank3")
test(keys, seed, "xxh64s,pow2,bdz,rank3")
test(keys, seed, "xxh64s,fastdiv,rank3")
test(keys, seed, "xxh64s,fastdiv,chm,rank3")
test(keys, seed, "xxh64s,fastdiv,bdz,rank3")
test(keys, seed, "xxh64s,pow2,fastdiv,rank3")
test(keys, seed, "xxh64s,pow2,fastdiv,chm,rank3")
test(keys, seed, "xxh64s,pow2,fastdiv,bdz,rank3")
