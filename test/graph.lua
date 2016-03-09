local rgph = require "rgph"

-- Compat stuff
if not bit32 then bit32 = bit end
if not bit32 then bit32 = require "bit" end

local args = { ... }
local seed = tonumber(args[1] or "123456789")

local function test_out_of_range(nkeys, flags)
	local ok, msg = pcall(rgph.new_graph, nkeys, flags)
	assert(not ok and msg:find("out of range"))
end

-- This function will calloc gigabytes of memory but it will not touch it.
local function test_good_range(nkeys, flags)
	assert(rgph.new_graph(nkeys, flags))
end

local rank2_max = { default=0x78787877, pow2=0x3c3c3c3c }
local rank3_max = { default=0xccccccca, pow2=0x99999999 }

test_good_range(rank2_max.default, "rank2")
test_good_range(rank2_max.pow2, "rank2,pow2")
test_good_range(rank3_max.default, "rank3")
test_good_range(rank3_max.pow2, "rank3,pow2")

test_out_of_range(0, "rank2")
test_out_of_range(0, "rank3")
test_out_of_range(rank2_max.default + 1, "rank2")
test_out_of_range(rank2_max.pow2 + 1, "rank2,pow2")
test_out_of_range(rank3_max.default + 1, "rank3")
test_out_of_range(rank3_max.pow2 + 1, "rank3,pow2")

local keys = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10,
               k=11, l=12, m=13, n=14, o=15, p=16, q=17, r=18,
               s=19, t=20, u=21, v=22, w=23, x=24, y=25, z=26 }

local function test_abcz(keys, seed, flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)
	local rank = g:rank()
	local nverts = g:vertices()
	local div_hint = g:division_hint()
	local div = nverts / rank

	flags = g:flags()

	assert(#flags > 0 and flags:find(div_hint))

	if div_hint == "pow2" then
		assert(not flags:find("fastdiv"))
		assert(bit32.band(div, div - 1) == 0)
	elseif div_hint == "fastdiv" then
		assert(not flags:find("pow2"))
		-- XXX check whether div is fastdiv
	end

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

test_abcz(keys, seed)

test_abcz(keys, seed, "jenkins2")
test_abcz(keys, seed, "jenkins2,chm")
test_abcz(keys, seed, "jenkins2,bdz")
test_abcz(keys, seed, "jenkins2,pow2")
test_abcz(keys, seed, "jenkins2,pow2,chm")
test_abcz(keys, seed, "jenkins2,pow2,bdz")
test_abcz(keys, seed, "jenkins2,fastdiv")
test_abcz(keys, seed, "jenkins2,fastdiv,chm")
test_abcz(keys, seed, "jenkins2,fastdiv,bdz")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,chm")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,bdz")
test_abcz(keys, seed, "jenkins2,rank2")
test_abcz(keys, seed, "jenkins2,chm,rank2")
test_abcz(keys, seed, "jenkins2,bdz,rank2")
test_abcz(keys, seed, "jenkins2,pow2,rank2")
test_abcz(keys, seed, "jenkins2,pow2,chm,rank2")
test_abcz(keys, seed, "jenkins2,pow2,bdz,rank2")
test_abcz(keys, seed, "jenkins2,fastdiv,rank2")
test_abcz(keys, seed, "jenkins2,fastdiv,chm,rank2")
test_abcz(keys, seed, "jenkins2,fastdiv,bdz,rank2")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,rank2")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,chm,rank2")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,bdz,rank2")
test_abcz(keys, seed, "jenkins2,rank3")
test_abcz(keys, seed, "jenkins2,chm,rank3")
test_abcz(keys, seed, "jenkins2,bdz,rank3")
test_abcz(keys, seed, "jenkins2,pow2,rank3")
test_abcz(keys, seed, "jenkins2,pow2,chm,rank3")
test_abcz(keys, seed, "jenkins2,pow2,bdz,rank3")
test_abcz(keys, seed, "jenkins2,fastdiv,rank3")
test_abcz(keys, seed, "jenkins2,fastdiv,chm,rank3")
test_abcz(keys, seed, "jenkins2,fastdiv,bdz,rank3")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,rank3")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,chm,rank3")
test_abcz(keys, seed, "jenkins2,pow2,fastdiv,bdz,rank3")

-- XXX "murmur32,rank3" tests hang
--test_abcz(keys, seed, "murmur32")
--test_abcz(keys, seed, "murmur32,chm")
--test_abcz(keys, seed, "murmur32,bdz")
test_abcz(keys, seed, "murmur32,pow2")
test_abcz(keys, seed, "murmur32,pow2,chm")
test_abcz(keys, seed, "murmur32,pow2,bdz")
--test_abcz(keys, seed, "murmur32,fastdiv")
--test_abcz(keys, seed, "murmur32,fastdiv,chm")
--test_abcz(keys, seed, "murmur32,fastdiv,bdz")
--test_abcz(keys, seed, "murmur32,pow2,fastdiv")
--test_abcz(keys, seed, "murmur32,pow2,fastdiv,chm")
--test_abcz(keys, seed, "murmur32,pow2,fastdiv,bdz")
test_abcz(keys, seed, "murmur32,rank2")
test_abcz(keys, seed, "murmur32,chm,rank2")
test_abcz(keys, seed, "murmur32,bdz,rank2")
test_abcz(keys, seed, "murmur32,pow2,rank2")
test_abcz(keys, seed, "murmur32,pow2,chm,rank2")
test_abcz(keys, seed, "murmur32,pow2,bdz,rank2")
test_abcz(keys, seed, "murmur32,fastdiv,rank2")
test_abcz(keys, seed, "murmur32,fastdiv,chm,rank2")
test_abcz(keys, seed, "murmur32,fastdiv,bdz,rank2")
test_abcz(keys, seed, "murmur32,pow2,fastdiv,rank2")
test_abcz(keys, seed, "murmur32,pow2,fastdiv,chm,rank2")
test_abcz(keys, seed, "murmur32,pow2,fastdiv,bdz,rank2")
--test_abcz(keys, seed, "murmur32,rank3")
--test_abcz(keys, seed, "murmur32,chm,rank3")
--test_abcz(keys, seed, "murmur32,bdz,rank3")
test_abcz(keys, seed, "murmur32,pow2,rank3")
test_abcz(keys, seed, "murmur32,pow2,chm,rank3")
test_abcz(keys, seed, "murmur32,pow2,bdz,rank3")
--test_abcz(keys, seed, "murmur32,fastdiv,rank3")
--test_abcz(keys, seed, "murmur32,fastdiv,chm,rank3")
--test_abcz(keys, seed, "murmur32,fastdiv,bdz,rank3")
--test_abcz(keys, seed, "murmur32,pow2,fastdiv,rank3")
--test_abcz(keys, seed, "murmur32,pow2,fastdiv,chm,rank3")
--test_abcz(keys, seed, "murmur32,pow2,fastdiv,bdz,rank3")

test_abcz(keys, seed, "murmur32s")
test_abcz(keys, seed, "murmur32s,chm")
test_abcz(keys, seed, "murmur32s,bdz")
test_abcz(keys, seed, "murmur32s,pow2")
test_abcz(keys, seed, "murmur32s,pow2,chm")
test_abcz(keys, seed, "murmur32s,pow2,bdz")
test_abcz(keys, seed, "murmur32s,fastdiv")
test_abcz(keys, seed, "murmur32s,fastdiv,chm")
test_abcz(keys, seed, "murmur32s,fastdiv,bdz")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,chm")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,bdz")
test_abcz(keys, seed, "murmur32s,rank2")
test_abcz(keys, seed, "murmur32s,chm,rank2")
test_abcz(keys, seed, "murmur32s,bdz,rank2")
test_abcz(keys, seed, "murmur32s,pow2,rank2")
test_abcz(keys, seed, "murmur32s,pow2,chm,rank2")
test_abcz(keys, seed, "murmur32s,pow2,bdz,rank2")
test_abcz(keys, seed, "murmur32s,fastdiv,rank2")
test_abcz(keys, seed, "murmur32s,fastdiv,chm,rank2")
test_abcz(keys, seed, "murmur32s,fastdiv,bdz,rank2")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,rank2")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,chm,rank2")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,bdz,rank2")
test_abcz(keys, seed, "murmur32s,rank3")
test_abcz(keys, seed, "murmur32s,chm,rank3")
test_abcz(keys, seed, "murmur32s,bdz,rank3")
test_abcz(keys, seed, "murmur32s,pow2,rank3")
test_abcz(keys, seed, "murmur32s,pow2,chm,rank3")
test_abcz(keys, seed, "murmur32s,pow2,bdz,rank3")
test_abcz(keys, seed, "murmur32s,fastdiv,rank3")
test_abcz(keys, seed, "murmur32s,fastdiv,chm,rank3")
test_abcz(keys, seed, "murmur32s,fastdiv,bdz,rank3")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,rank3")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,chm,rank3")
test_abcz(keys, seed, "murmur32s,pow2,fastdiv,bdz,rank3")

test_abcz(keys, seed, "xxh32s")
test_abcz(keys, seed, "xxh32s,chm")
test_abcz(keys, seed, "xxh32s,bdz")
test_abcz(keys, seed, "xxh32s,pow2")
test_abcz(keys, seed, "xxh32s,pow2,chm")
test_abcz(keys, seed, "xxh32s,pow2,bdz")
test_abcz(keys, seed, "xxh32s,fastdiv")
test_abcz(keys, seed, "xxh32s,fastdiv,chm")
test_abcz(keys, seed, "xxh32s,fastdiv,bdz")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,chm")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,bdz")
test_abcz(keys, seed, "xxh32s,rank2")
test_abcz(keys, seed, "xxh32s,chm,rank2")
test_abcz(keys, seed, "xxh32s,bdz,rank2")
test_abcz(keys, seed, "xxh32s,pow2,rank2")
test_abcz(keys, seed, "xxh32s,pow2,chm,rank2")
test_abcz(keys, seed, "xxh32s,pow2,bdz,rank2")
test_abcz(keys, seed, "xxh32s,fastdiv,rank2")
test_abcz(keys, seed, "xxh32s,fastdiv,chm,rank2")
test_abcz(keys, seed, "xxh32s,fastdiv,bdz,rank2")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,rank2")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,chm,rank2")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,bdz,rank2")
test_abcz(keys, seed, "xxh32s,rank3")
test_abcz(keys, seed, "xxh32s,chm,rank3")
test_abcz(keys, seed, "xxh32s,bdz,rank3")
test_abcz(keys, seed, "xxh32s,pow2,rank3")
test_abcz(keys, seed, "xxh32s,pow2,chm,rank3")
test_abcz(keys, seed, "xxh32s,pow2,bdz,rank3")
test_abcz(keys, seed, "xxh32s,fastdiv,rank3")
test_abcz(keys, seed, "xxh32s,fastdiv,chm,rank3")
test_abcz(keys, seed, "xxh32s,fastdiv,bdz,rank3")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,rank3")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,chm,rank3")
test_abcz(keys, seed, "xxh32s,pow2,fastdiv,bdz,rank3")

test_abcz(keys, seed, "xxh64s")
test_abcz(keys, seed, "xxh64s,chm")
test_abcz(keys, seed, "xxh64s,bdz")
test_abcz(keys, seed, "xxh64s,pow2")
test_abcz(keys, seed, "xxh64s,pow2,chm")
test_abcz(keys, seed, "xxh64s,pow2,bdz")
test_abcz(keys, seed, "xxh64s,fastdiv")
test_abcz(keys, seed, "xxh64s,fastdiv,chm")
test_abcz(keys, seed, "xxh64s,fastdiv,bdz")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,chm")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,bdz")
test_abcz(keys, seed, "xxh64s,rank2")
test_abcz(keys, seed, "xxh64s,chm,rank2")
test_abcz(keys, seed, "xxh64s,bdz,rank2")
test_abcz(keys, seed, "xxh64s,pow2,rank2")
test_abcz(keys, seed, "xxh64s,pow2,chm,rank2")
test_abcz(keys, seed, "xxh64s,pow2,bdz,rank2")
test_abcz(keys, seed, "xxh64s,fastdiv,rank2")
test_abcz(keys, seed, "xxh64s,fastdiv,chm,rank2")
test_abcz(keys, seed, "xxh64s,fastdiv,bdz,rank2")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,rank2")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,chm,rank2")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,bdz,rank2")
test_abcz(keys, seed, "xxh64s,rank3")
test_abcz(keys, seed, "xxh64s,chm,rank3")
test_abcz(keys, seed, "xxh64s,bdz,rank3")
test_abcz(keys, seed, "xxh64s,pow2,rank3")
test_abcz(keys, seed, "xxh64s,pow2,chm,rank3")
test_abcz(keys, seed, "xxh64s,pow2,bdz,rank3")
test_abcz(keys, seed, "xxh64s,fastdiv,rank3")
test_abcz(keys, seed, "xxh64s,fastdiv,chm,rank3")
test_abcz(keys, seed, "xxh64s,fastdiv,bdz,rank3")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,rank3")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,chm,rank3")
test_abcz(keys, seed, "xxh64s,pow2,fastdiv,bdz,rank3")
