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
local function test_good_range(nkeys, flags, reduction)
	local g = assert(rgph.new_graph(nkeys, flags))
	assert(g:reduction() == reduction)
end

local rank2_max = { vec = 0x78787877, s64 = 0x78787877, s32 = 0x00007878 }
local rank3_max = { vec = 0xccccccca, s64 = 0x00266666, s32 = 0x000004cc }

-- rank2 vector hash
test_out_of_range(0, "jenkins2v,rank2")
test_out_of_range(0, "jenkins2v,rank2,mod")
test_out_of_range(0, "jenkins2v,rank2,mul")

test_good_range(rank2_max.vec, "jenkins2v,rank2", "mod")
test_good_range(rank2_max.vec, "jenkins2v,rank2,mod", "mod")
test_good_range(rank2_max.vec, "jenkins2v,rank2,mul", "mul")

test_out_of_range(rank2_max.vec + 1, "jenkins2v,rank2")
test_out_of_range(rank2_max.vec + 1, "jenkins2v,rank2,mod")
test_out_of_range(rank2_max.vec + 1, "jenkins2v,rank2,mul")

-- rank2 scalar 64 hash
test_out_of_range(0, "xxh64s,rank2")
test_out_of_range(0, "xxh64s,rank2,mod")
test_out_of_range(0, "xxh64s,rank2,mul")

test_good_range(rank2_max.s64, "xxh64s,rank2", "mod")
test_good_range(rank2_max.s64, "xxh64s,rank2,mod", "mod")
test_good_range(rank2_max.s64, "xxh64s,rank2,mul", "mul")

test_out_of_range(rank2_max.s64 + 1, "xxh64s,rank2")
test_out_of_range(rank2_max.s64 + 1, "xxh64s,rank2,mod")
test_out_of_range(rank2_max.s64 + 1, "xxh64s,rank2,mul")

-- rank2 scalar 32 hash
test_out_of_range(0, "xxh32s,rank2")
test_out_of_range(0, "xxh32s,rank2,mod")
test_out_of_range(0, "xxh32s,rank2,mul")

test_good_range(rank2_max.s32, "xxh32s,rank2", "mod")
test_good_range(rank2_max.s32, "xxh32s,rank2,mod", "mod")
test_good_range(rank2_max.s32, "xxh32s,rank2,mul", "mul")

-- rank3 vector hash
test_out_of_range(0, "jenkins2v,rank3")
test_out_of_range(0, "jenkins2v,rank3,mod")
test_out_of_range(0, "jenkins2v,rank3,mul")

test_good_range(rank3_max.vec, "jenkins2v,rank3", "mod")
test_good_range(rank3_max.vec, "jenkins2v,rank3,mod", "mod")
test_good_range(rank3_max.vec, "jenkins2v,rank3,mul", "mul")

test_out_of_range(rank3_max.vec + 1, "jenkins2v,rank3")
test_out_of_range(rank3_max.vec + 1, "jenkins2v,rank3,mod")
test_out_of_range(rank3_max.vec + 1, "jenkins2v,rank3,mul")

-- rank3 scalar 64 hash
test_out_of_range(0, "xxh64s,rank3")
test_out_of_range(0, "xxh64s,rank3,mod")
test_out_of_range(0, "xxh64s,rank3,mul")

test_good_range(rank3_max.s64, "xxh64s,rank3", "mod")
test_good_range(rank3_max.s64, "xxh64s,rank3,mod", "mod")
test_good_range(rank3_max.s64, "xxh64s,rank3,mul", "mul")

test_out_of_range(rank3_max.s64 + 1, "xxh64s,rank3")
test_out_of_range(rank3_max.s64 + 1, "xxh64s,rank3,mod")
test_out_of_range(rank3_max.s64 + 1, "xxh64s,rank3,mul")

-- rank3 scalar 32 hash
test_out_of_range(0, "xxh32s,rank3")
test_out_of_range(0, "xxh32s,rank3,mod")
test_out_of_range(0, "xxh32s,rank3,mul")

test_good_range(rank3_max.s32, "xxh32s,rank3", "mod")
test_good_range(rank3_max.s32, "xxh32s,rank3,mod", "mod")
test_good_range(rank3_max.s32, "xxh32s,rank3,mul", "mul")

test_out_of_range(rank3_max.s32 + 1, "xxh32s,rank3")
test_out_of_range(rank3_max.s32 + 1, "xxh32s,rank3,mod")
test_out_of_range(rank3_max.s32 + 1, "xxh32s,rank3,mul")


local abcz = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10,
               k=11, l=12, m=13, n=14, o=15, p=16, q=17, r=18,
               s=19, t=20, u=21, v=22, w=23, x=24, y=25, z=26 }

local function test_abcz(keys, seed, flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)
	local rank = g:rank()
	local nverts = g:vertices()
	local div = nverts / rank

	flags = g:flags()

	assert(#flags > 0 and flags:find(g:reduction()))

	while true do
		local ok, err = g:build(seed, pairs(keys))
		if ok then break end
		assert(not err, err) -- pairs(keys) can't fail
		seed = seed + 1
	end

	-- Numbers are converted to strings by Lua
	assert(g:datalen_min() == 1)
	assert(g:datalen_max() == 2)

	local bdz = g:algo() == "bdz"
	local unassigned = bdz and rank or nkeys

	local assign = assert(g:assign(bdz and "bdz" or "chm,compact"))

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

test_abcz(abcz, seed)

test_abcz(abcz, seed, "jenkins2v")
test_abcz(abcz, seed, "jenkins2v,chm")
test_abcz(abcz, seed, "jenkins2v,bdz")
test_abcz(abcz, seed, "jenkins2v,mod")
test_abcz(abcz, seed, "jenkins2v,mod,chm")
test_abcz(abcz, seed, "jenkins2v,mod,bdz")
-- XXX these tests hang:
--test_abcz(abcz, seed, "jenkins2v,mul")
--test_abcz(abcz, seed, "jenkins2v,mul,chm")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz")
test_abcz(abcz, seed, "jenkins2v,rank2")
test_abcz(abcz, seed, "jenkins2v,chm,rank2")
test_abcz(abcz, seed, "jenkins2v,bdz,rank2")
test_abcz(abcz, seed, "jenkins2v,mod,rank2")
test_abcz(abcz, seed, "jenkins2v,mod,chm,rank2")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,rank2")
-- XXX these tests are slow:
--test_abcz(abcz, seed, "jenkins2v,mul,rank2")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,rank2")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,rank2")
test_abcz(abcz, seed, "jenkins2v,rank3")
test_abcz(abcz, seed, "jenkins2v,chm,rank3")
test_abcz(abcz, seed, "jenkins2v,bdz,rank3")
test_abcz(abcz, seed, "jenkins2v,mod,rank3")
test_abcz(abcz, seed, "jenkins2v,mod,chm,rank3")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,rank3")
-- XXX these tests hang:
--test_abcz(abcz, seed, "jenkins2v,mul,rank3")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,rank3")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,rank3")

-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v")
--test_abcz(abcz, seed, "murmur32v,chm")
--test_abcz(abcz, seed, "murmur32v,bdz")
--test_abcz(abcz, seed, "murmur32v,mod")
--test_abcz(abcz, seed, "murmur32v,mod,chm")
--test_abcz(abcz, seed, "murmur32v,mod,bdz")
-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,mul")
--test_abcz(abcz, seed, "murmur32v,mul,chm")
--test_abcz(abcz, seed, "murmur32v,mul,bdz")
test_abcz(abcz, seed, "murmur32v,rank2")
test_abcz(abcz, seed, "murmur32v,chm,rank2")
test_abcz(abcz, seed, "murmur32v,bdz,rank2")
test_abcz(abcz, seed, "murmur32v,mod,rank2")
test_abcz(abcz, seed, "murmur32v,mod,chm,rank2")
test_abcz(abcz, seed, "murmur32v,mod,bdz,rank2")
test_abcz(abcz, seed, "murmur32v,mul,rank2")
test_abcz(abcz, seed, "murmur32v,mul,chm,rank2")
test_abcz(abcz, seed, "murmur32v,mul,bdz,rank2")
--test_abcz(abcz, seed, "murmur32v,rank3")
--test_abcz(abcz, seed, "murmur32v,chm,rank3")
--test_abcz(abcz, seed, "murmur32v,bdz,rank3")
--test_abcz(abcz, seed, "murmur32v,mod,rank3")
--test_abcz(abcz, seed, "murmur32v,mod,chm,rank3")
--test_abcz(abcz, seed, "murmur32v,mod,bdz,rank3")
-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,mul,rank3")
--test_abcz(abcz, seed, "murmur32v,mul,chm,rank3")
--test_abcz(abcz, seed, "murmur32v,mul,bdz,rank3")

test_abcz(abcz, seed, "murmur32s")
test_abcz(abcz, seed, "murmur32s,chm")
test_abcz(abcz, seed, "murmur32s,bdz")
test_abcz(abcz, seed, "murmur32s,mod")
test_abcz(abcz, seed, "murmur32s,mod,chm")
test_abcz(abcz, seed, "murmur32s,mod,bdz")
test_abcz(abcz, seed, "murmur32s,mul")
test_abcz(abcz, seed, "murmur32s,mul,chm")
test_abcz(abcz, seed, "murmur32s,mul,bdz")
test_abcz(abcz, seed, "murmur32s,rank2")
test_abcz(abcz, seed, "murmur32s,chm,rank2")
test_abcz(abcz, seed, "murmur32s,bdz,rank2")
test_abcz(abcz, seed, "murmur32s,mod,rank2")
test_abcz(abcz, seed, "murmur32s,mod,chm,rank2")
test_abcz(abcz, seed, "murmur32s,mod,bdz,rank2")
test_abcz(abcz, seed, "murmur32s,mul,rank2")
test_abcz(abcz, seed, "murmur32s,mul,chm,rank2")
test_abcz(abcz, seed, "murmur32s,mul,bdz,rank2")
test_abcz(abcz, seed, "murmur32s,rank3")
test_abcz(abcz, seed, "murmur32s,chm,rank3")
test_abcz(abcz, seed, "murmur32s,bdz,rank3")
test_abcz(abcz, seed, "murmur32s,mod,rank3")
test_abcz(abcz, seed, "murmur32s,mod,chm,rank3")
test_abcz(abcz, seed, "murmur32s,mod,bdz,rank3")
test_abcz(abcz, seed, "murmur32s,mul,rank3")
test_abcz(abcz, seed, "murmur32s,mul,chm,rank3")
test_abcz(abcz, seed, "murmur32s,mul,bdz,rank3")

test_abcz(abcz, seed, "xxh32s")
test_abcz(abcz, seed, "xxh32s,chm")
test_abcz(abcz, seed, "xxh32s,bdz")
test_abcz(abcz, seed, "xxh32s,mod")
test_abcz(abcz, seed, "xxh32s,mod,chm")
test_abcz(abcz, seed, "xxh32s,mod,bdz")
test_abcz(abcz, seed, "xxh32s,mul")
test_abcz(abcz, seed, "xxh32s,mul,chm")
test_abcz(abcz, seed, "xxh32s,mul,bdz")
test_abcz(abcz, seed, "xxh32s,rank2")
test_abcz(abcz, seed, "xxh32s,chm,rank2")
test_abcz(abcz, seed, "xxh32s,bdz,rank2")
test_abcz(abcz, seed, "xxh32s,mod,rank2")
test_abcz(abcz, seed, "xxh32s,mod,chm,rank2")
test_abcz(abcz, seed, "xxh32s,mod,bdz,rank2")
test_abcz(abcz, seed, "xxh32s,mul,rank2")
test_abcz(abcz, seed, "xxh32s,mul,chm,rank2")
test_abcz(abcz, seed, "xxh32s,mul,bdz,rank2")
test_abcz(abcz, seed, "xxh32s,rank3")
test_abcz(abcz, seed, "xxh32s,chm,rank3")
test_abcz(abcz, seed, "xxh32s,bdz,rank3")
test_abcz(abcz, seed, "xxh32s,mod,rank3")
test_abcz(abcz, seed, "xxh32s,mod,chm,rank3")
test_abcz(abcz, seed, "xxh32s,mod,bdz,rank3")
test_abcz(abcz, seed, "xxh32s,mul,rank3")
test_abcz(abcz, seed, "xxh32s,mul,chm,rank3")
test_abcz(abcz, seed, "xxh32s,mul,bdz,rank3")

test_abcz(abcz, seed, "xxh64s")
test_abcz(abcz, seed, "xxh64s,chm")
test_abcz(abcz, seed, "xxh64s,bdz")
test_abcz(abcz, seed, "xxh64s,mod")
test_abcz(abcz, seed, "xxh64s,mod,chm")
test_abcz(abcz, seed, "xxh64s,mod,bdz")
test_abcz(abcz, seed, "xxh64s,mul")
test_abcz(abcz, seed, "xxh64s,mul,chm")
test_abcz(abcz, seed, "xxh64s,mul,bdz")
test_abcz(abcz, seed, "xxh64s,rank2")
test_abcz(abcz, seed, "xxh64s,chm,rank2")
test_abcz(abcz, seed, "xxh64s,bdz,rank2")
test_abcz(abcz, seed, "xxh64s,mod,rank2")
test_abcz(abcz, seed, "xxh64s,mod,chm,rank2")
test_abcz(abcz, seed, "xxh64s,mod,bdz,rank2")
test_abcz(abcz, seed, "xxh64s,mul,rank2")
test_abcz(abcz, seed, "xxh64s,mul,chm,rank2")
test_abcz(abcz, seed, "xxh64s,mul,bdz,rank2")
test_abcz(abcz, seed, "xxh64s,rank3")
test_abcz(abcz, seed, "xxh64s,chm,rank3")
test_abcz(abcz, seed, "xxh64s,bdz,rank3")
test_abcz(abcz, seed, "xxh64s,mod,rank3")
test_abcz(abcz, seed, "xxh64s,mod,chm,rank3")
test_abcz(abcz, seed, "xxh64s,mod,bdz,rank3")
test_abcz(abcz, seed, "xxh64s,mul,rank3")
test_abcz(abcz, seed, "xxh64s,mul,chm,rank3")
test_abcz(abcz, seed, "xxh64s,mul,bdz,rank3")
