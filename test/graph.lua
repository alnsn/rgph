local rgph = require "rgph"

-- Compat stuff
if not bit32 then bit32 = bit end
if not bit32 then bit32 = require "bit" end

local args = { ... }
local seed = tonumber(args[1] or "123456789")

local rank2_max = { vec = 0x78787877, s64 = 0x78787877, s32 = 0x00007878 }
local rank3_max = { vec = 0xccccccca, s64 = 0x00266666, s32 = 0x000004cc }

local function test_out_of_range(nkeys, flags)
	local ok, msg = pcall(rgph.new_graph, nkeys, flags)
	assert(not ok and msg:find("out of range"))
end

-- This function will calloc gigabytes of memory but it will not touch it.
local function test_good_range(nkeys, flags, reduction)
	local g = assert(rgph.new_graph(nkeys, flags))
	assert(g:reduction() == reduction)
end

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


local function align_up_pow2(n)
	-- Lua 5.3 returns these constants as integers.
	if n >= 4611686018427387904 then return -1 -- Out of range.
	elseif n >=  2305843009213693952 then return 4611686018427387904 -- 2^62
	elseif n >=  1152921504606846976 then return 2305843009213693952 -- 2^61
	elseif n >=  576460752303423488  then return 1152921504606846976 -- 2^60
	elseif n >=  288230376151711744  then return 576460752303423488 -- 2^59
	elseif n >=  144115188075855872  then return 288230376151711744 -- 2^58
	elseif n >=  72057594037927936   then return 144115188075855872 -- 2^57
	elseif n >=  36028797018963968   then return 72057594037927936 -- 2^56
	elseif n >=  18014398509481984   then return 36028797018963968 -- 2^55
	elseif n >=  9007199254740992    then return 18014398509481984 -- 2^54
	elseif n >=  4503599627370496    then return 9007199254740992 -- 2^53
	elseif n >=  2251799813685248    then return 4503599627370496 -- 2^52
	elseif n >=  1125899906842624    then return 2251799813685248 -- 2^51
	elseif n >=  562949953421312     then return 1125899906842624 -- 2^50
	elseif n >=  281474976710656     then return 562949953421312 -- 2^49
	elseif n >=  140737488355328     then return 281474976710656 -- 2^48
	elseif n >=  70368744177664      then return 140737488355328 -- 2^47
	elseif n >=  35184372088832      then return 70368744177664 -- 2^46
	elseif n >=  17592186044416      then return 35184372088832 -- 2^45
	elseif n >=  8796093022208       then return 17592186044416 -- 2^44
	elseif n >=  4398046511104       then return 8796093022208 -- 2^43
	elseif n >=  2199023255552       then return 4398046511104 -- 2^42
	elseif n >=  1099511627776       then return 2199023255552 -- 2^41
	elseif n >=  549755813888        then return 1099511627776 -- 2^40
	elseif n >=  274877906944        then return 549755813888 -- 2^39
	elseif n >=  137438953472        then return 274877906944 -- 2^38
	elseif n >=  68719476736         then return 137438953472 -- 2^37
	elseif n >=  34359738368         then return 68719476736 -- 2^36
	elseif n >=  17179869184         then return 34359738368 -- 2^35
	elseif n >=  8589934592          then return 17179869184 -- 2^34
	elseif n >=  4294967296          then return 8589934592 -- 2^33
	end

	local r = 1
	while n > 0 do
		n = bit32.rshift(n, 1)
		r = 2 * r
	end
	return r
end


local abcz = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, j=10,
               k=11, l=12, m=13, n=14, o=15, p=16, q=17, r=18,
               s=19, t=20, u=21, v=22, w=23, x=24, y=25, z=26 }

local function test_abcz(keys, seed, flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)
	local rank = g:rank()
	local nverts = g:vertices()

	flags = g:flags()

	assert(#flags > 0 and flags:find(g:reduction()))

	while true do
		local ok, err = g:build(nil, seed, pairs(keys))
		if ok then break end
		assert(not err, err) -- pairs(keys) can't fail
		seed = seed + 1
	end

	-- Numbers are converted to strings by Lua
	assert(g:datalen_min() == 1)
	assert(g:datalen_max() == 2)

	assert(g:index_min() == 0)
	assert(g:index_max() == nkeys - 1)

	local bdz = g:algo() == "bdz"
	local unassigned = bdz and rank or
	    (g:index() == "compact" and nkeys or align_up_pow2(nkeys))

	local assign = assert(g:assign())

	assert(g:index_min() == 0)
	assert(g:index_max() == nkeys - 1)

	for v = 0, nverts - 1 do
		assert(assign[v] >= 0 and assign[v] < unassigned)
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

test_abcz(abcz, seed, "jenkins2v,compact")
test_abcz(abcz, seed, "jenkins2v,chm,compact")
test_abcz(abcz, seed, "jenkins2v,bdz,compact")
test_abcz(abcz, seed, "jenkins2v,mod,compact")
test_abcz(abcz, seed, "jenkins2v,mod,chm,compact")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,compact")
-- XXX these tests hang:
--test_abcz(abcz, seed, "jenkins2v,mul,compact")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,compact")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,compact")
test_abcz(abcz, seed, "jenkins2v,rank2,compact")
test_abcz(abcz, seed, "jenkins2v,chm,rank2,compact")
test_abcz(abcz, seed, "jenkins2v,bdz,rank2,compact")
test_abcz(abcz, seed, "jenkins2v,mod,rank2,compact")
test_abcz(abcz, seed, "jenkins2v,mod,chm,rank2,compact")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,rank2,compact")
-- XXX these tests are slow:
--test_abcz(abcz, seed, "jenkins2v,mul,rank2,compact")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,rank2,compact")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,rank2,compact")
test_abcz(abcz, seed, "jenkins2v,rank3,compact")
test_abcz(abcz, seed, "jenkins2v,chm,rank3,compact")
test_abcz(abcz, seed, "jenkins2v,bdz,rank3,compact")
test_abcz(abcz, seed, "jenkins2v,mod,rank3,compact")
test_abcz(abcz, seed, "jenkins2v,mod,chm,rank3,compact")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,rank3,compact")
-- XXX these tests hang:
--test_abcz(abcz, seed, "jenkins2v,mul,rank3,compact")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,rank3,compact")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,rank3,compact")

test_abcz(abcz, seed, "jenkins2v,sparse")
test_abcz(abcz, seed, "jenkins2v,chm,sparse")
test_abcz(abcz, seed, "jenkins2v,bdz,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,chm,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,sparse")
-- XXX these tests hang:
--test_abcz(abcz, seed, "jenkins2v,mul,sparse")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,sparse")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,sparse")
test_abcz(abcz, seed, "jenkins2v,rank2,sparse")
test_abcz(abcz, seed, "jenkins2v,chm,rank2,sparse")
test_abcz(abcz, seed, "jenkins2v,bdz,rank2,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,rank2,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,chm,rank2,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,rank2,sparse")
-- XXX these tests are slow:
--test_abcz(abcz, seed, "jenkins2v,mul,rank2,sparse")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,rank2,sparse")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,rank2,sparse")
test_abcz(abcz, seed, "jenkins2v,rank3,sparse")
test_abcz(abcz, seed, "jenkins2v,chm,rank3,sparse")
test_abcz(abcz, seed, "jenkins2v,bdz,rank3,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,rank3,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,chm,rank3,sparse")
test_abcz(abcz, seed, "jenkins2v,mod,bdz,rank3,sparse")
-- XXX these tests hang:
--test_abcz(abcz, seed, "jenkins2v,mul,rank3,sparse")
--test_abcz(abcz, seed, "jenkins2v,mul,chm,rank3,sparse")
--test_abcz(abcz, seed, "jenkins2v,mul,bdz,rank3,sparse")

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

-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,compact")
--test_abcz(abcz, seed, "murmur32v,chm,compact")
--test_abcz(abcz, seed, "murmur32v,bdz,compact")
--test_abcz(abcz, seed, "murmur32v,mod,compact")
--test_abcz(abcz, seed, "murmur32v,mod,chm,compact")
--test_abcz(abcz, seed, "murmur32v,mod,bdz,compact")
-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,mul,compact")
--test_abcz(abcz, seed, "murmur32v,mul,chm,compact")
--test_abcz(abcz, seed, "murmur32v,mul,bdz,compact")
test_abcz(abcz, seed, "murmur32v,rank2,compact")
test_abcz(abcz, seed, "murmur32v,chm,rank2,compact")
test_abcz(abcz, seed, "murmur32v,bdz,rank2,compact")
test_abcz(abcz, seed, "murmur32v,mod,rank2,compact")
test_abcz(abcz, seed, "murmur32v,mod,chm,rank2,compact")
test_abcz(abcz, seed, "murmur32v,mod,bdz,rank2,compact")
test_abcz(abcz, seed, "murmur32v,mul,rank2,compact")
test_abcz(abcz, seed, "murmur32v,mul,chm,rank2,compact")
test_abcz(abcz, seed, "murmur32v,mul,bdz,rank2,compact")
--test_abcz(abcz, seed, "murmur32v,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,chm,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,bdz,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,mod,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,mod,chm,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,mod,bdz,rank3,compact")
-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,mul,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,mul,chm,rank3,compact")
--test_abcz(abcz, seed, "murmur32v,mul,bdz,rank3,compact")

-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,sparse")
--test_abcz(abcz, seed, "murmur32v,chm,sparse")
--test_abcz(abcz, seed, "murmur32v,bdz,sparse")
--test_abcz(abcz, seed, "murmur32v,mod,sparse")
--test_abcz(abcz, seed, "murmur32v,mod,chm,sparse")
--test_abcz(abcz, seed, "murmur32v,mod,bdz,sparse")
-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,mul,sparse")
--test_abcz(abcz, seed, "murmur32v,mul,chm,sparse")
--test_abcz(abcz, seed, "murmur32v,mul,bdz,sparse")
test_abcz(abcz, seed, "murmur32v,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,chm,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,bdz,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,mod,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,mod,chm,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,mod,bdz,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,mul,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,mul,chm,rank2,sparse")
test_abcz(abcz, seed, "murmur32v,mul,bdz,rank2,sparse")
--test_abcz(abcz, seed, "murmur32v,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,chm,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,bdz,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,mod,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,mod,chm,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,mod,bdz,rank3,sparse")
-- XXX these tests hang:
--test_abcz(abcz, seed, "murmur32v,mul,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,mul,chm,rank3,sparse")
--test_abcz(abcz, seed, "murmur32v,mul,bdz,rank3,sparse")

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

test_abcz(abcz, seed, "murmur32s,compact")
test_abcz(abcz, seed, "murmur32s,chm,compact")
test_abcz(abcz, seed, "murmur32s,bdz,compact")
test_abcz(abcz, seed, "murmur32s,mod,compact")
test_abcz(abcz, seed, "murmur32s,mod,chm,compact")
test_abcz(abcz, seed, "murmur32s,mod,bdz,compact")
test_abcz(abcz, seed, "murmur32s,mul,compact")
test_abcz(abcz, seed, "murmur32s,mul,chm,compact")
test_abcz(abcz, seed, "murmur32s,mul,bdz,compact")
test_abcz(abcz, seed, "murmur32s,rank2,compact")
test_abcz(abcz, seed, "murmur32s,chm,rank2,compact")
test_abcz(abcz, seed, "murmur32s,bdz,rank2,compact")
test_abcz(abcz, seed, "murmur32s,mod,rank2,compact")
test_abcz(abcz, seed, "murmur32s,mod,chm,rank2,compact")
test_abcz(abcz, seed, "murmur32s,mod,bdz,rank2,compact")
test_abcz(abcz, seed, "murmur32s,mul,rank2,compact")
test_abcz(abcz, seed, "murmur32s,mul,chm,rank2,compact")
test_abcz(abcz, seed, "murmur32s,mul,bdz,rank2,compact")
test_abcz(abcz, seed, "murmur32s,rank3,compact")
test_abcz(abcz, seed, "murmur32s,chm,rank3,compact")
test_abcz(abcz, seed, "murmur32s,bdz,rank3,compact")
test_abcz(abcz, seed, "murmur32s,mod,rank3,compact")
test_abcz(abcz, seed, "murmur32s,mod,chm,rank3,compact")
test_abcz(abcz, seed, "murmur32s,mod,bdz,rank3,compact")
test_abcz(abcz, seed, "murmur32s,mul,rank3,compact")
test_abcz(abcz, seed, "murmur32s,mul,chm,rank3,compact")
test_abcz(abcz, seed, "murmur32s,mul,bdz,rank3,compact")

test_abcz(abcz, seed, "murmur32s,sparse")
test_abcz(abcz, seed, "murmur32s,chm,sparse")
test_abcz(abcz, seed, "murmur32s,bdz,sparse")
test_abcz(abcz, seed, "murmur32s,mod,sparse")
test_abcz(abcz, seed, "murmur32s,mod,chm,sparse")
test_abcz(abcz, seed, "murmur32s,mod,bdz,sparse")
test_abcz(abcz, seed, "murmur32s,mul,sparse")
test_abcz(abcz, seed, "murmur32s,mul,chm,sparse")
test_abcz(abcz, seed, "murmur32s,mul,bdz,sparse")
test_abcz(abcz, seed, "murmur32s,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,chm,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,bdz,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,mod,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,mod,chm,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,mod,bdz,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,mul,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,mul,chm,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,mul,bdz,rank2,sparse")
test_abcz(abcz, seed, "murmur32s,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,chm,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,bdz,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,mod,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,mod,chm,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,mod,bdz,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,mul,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,mul,chm,rank3,sparse")
test_abcz(abcz, seed, "murmur32s,mul,bdz,rank3,sparse")

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

test_abcz(abcz, seed, "xxh32s,compact")
test_abcz(abcz, seed, "xxh32s,chm,compact")
test_abcz(abcz, seed, "xxh32s,bdz,compact")
test_abcz(abcz, seed, "xxh32s,mod,compact")
test_abcz(abcz, seed, "xxh32s,mod,chm,compact")
test_abcz(abcz, seed, "xxh32s,mod,bdz,compact")
test_abcz(abcz, seed, "xxh32s,mul,compact")
test_abcz(abcz, seed, "xxh32s,mul,chm,compact")
test_abcz(abcz, seed, "xxh32s,mul,bdz,compact")
test_abcz(abcz, seed, "xxh32s,rank2,compact")
test_abcz(abcz, seed, "xxh32s,chm,rank2,compact")
test_abcz(abcz, seed, "xxh32s,bdz,rank2,compact")
test_abcz(abcz, seed, "xxh32s,mod,rank2,compact")
test_abcz(abcz, seed, "xxh32s,mod,chm,rank2,compact")
test_abcz(abcz, seed, "xxh32s,mod,bdz,rank2,compact")
test_abcz(abcz, seed, "xxh32s,mul,rank2,compact")
test_abcz(abcz, seed, "xxh32s,mul,chm,rank2,compact")
test_abcz(abcz, seed, "xxh32s,mul,bdz,rank2,compact")
test_abcz(abcz, seed, "xxh32s,rank3,compact")
test_abcz(abcz, seed, "xxh32s,chm,rank3,compact")
test_abcz(abcz, seed, "xxh32s,bdz,rank3,compact")
test_abcz(abcz, seed, "xxh32s,mod,rank3,compact")
test_abcz(abcz, seed, "xxh32s,mod,chm,rank3,compact")
test_abcz(abcz, seed, "xxh32s,mod,bdz,rank3,compact")
test_abcz(abcz, seed, "xxh32s,mul,rank3,compact")
test_abcz(abcz, seed, "xxh32s,mul,chm,rank3,compact")
test_abcz(abcz, seed, "xxh32s,mul,bdz,rank3,compact")

test_abcz(abcz, seed, "xxh32s,sparse")
test_abcz(abcz, seed, "xxh32s,chm,sparse")
test_abcz(abcz, seed, "xxh32s,bdz,sparse")
test_abcz(abcz, seed, "xxh32s,mod,sparse")
test_abcz(abcz, seed, "xxh32s,mod,chm,sparse")
test_abcz(abcz, seed, "xxh32s,mod,bdz,sparse")
test_abcz(abcz, seed, "xxh32s,mul,sparse")
test_abcz(abcz, seed, "xxh32s,mul,chm,sparse")
test_abcz(abcz, seed, "xxh32s,mul,bdz,sparse")
test_abcz(abcz, seed, "xxh32s,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,chm,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,bdz,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,mod,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,mod,chm,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,mod,bdz,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,mul,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,mul,chm,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,mul,bdz,rank2,sparse")
test_abcz(abcz, seed, "xxh32s,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,chm,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,bdz,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,mod,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,mod,chm,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,mod,bdz,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,mul,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,mul,chm,rank3,sparse")
test_abcz(abcz, seed, "xxh32s,mul,bdz,rank3,sparse")

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

test_abcz(abcz, seed, "xxh64s,compact")
test_abcz(abcz, seed, "xxh64s,chm,compact")
test_abcz(abcz, seed, "xxh64s,bdz,compact")
test_abcz(abcz, seed, "xxh64s,mod,compact")
test_abcz(abcz, seed, "xxh64s,mod,chm,compact")
test_abcz(abcz, seed, "xxh64s,mod,bdz,compact")
test_abcz(abcz, seed, "xxh64s,mul,compact")
test_abcz(abcz, seed, "xxh64s,mul,chm,compact")
test_abcz(abcz, seed, "xxh64s,mul,bdz,compact")
test_abcz(abcz, seed, "xxh64s,rank2,compact")
test_abcz(abcz, seed, "xxh64s,chm,rank2,compact")
test_abcz(abcz, seed, "xxh64s,bdz,rank2,compact")
test_abcz(abcz, seed, "xxh64s,mod,rank2,compact")
test_abcz(abcz, seed, "xxh64s,mod,chm,rank2,compact")
test_abcz(abcz, seed, "xxh64s,mod,bdz,rank2,compact")
test_abcz(abcz, seed, "xxh64s,mul,rank2,compact")
test_abcz(abcz, seed, "xxh64s,mul,chm,rank2,compact")
test_abcz(abcz, seed, "xxh64s,mul,bdz,rank2,compact")
test_abcz(abcz, seed, "xxh64s,rank3,compact")
test_abcz(abcz, seed, "xxh64s,chm,rank3,compact")
test_abcz(abcz, seed, "xxh64s,bdz,rank3,compact")
test_abcz(abcz, seed, "xxh64s,mod,rank3,compact")
test_abcz(abcz, seed, "xxh64s,mod,chm,rank3,compact")
test_abcz(abcz, seed, "xxh64s,mod,bdz,rank3,compact")
test_abcz(abcz, seed, "xxh64s,mul,rank3,compact")
test_abcz(abcz, seed, "xxh64s,mul,chm,rank3,compact")
test_abcz(abcz, seed, "xxh64s,mul,bdz,rank3,compact")

test_abcz(abcz, seed, "xxh64s,sparse")
test_abcz(abcz, seed, "xxh64s,chm,sparse")
test_abcz(abcz, seed, "xxh64s,bdz,sparse")
test_abcz(abcz, seed, "xxh64s,mod,sparse")
test_abcz(abcz, seed, "xxh64s,mod,chm,sparse")
test_abcz(abcz, seed, "xxh64s,mod,bdz,sparse")
test_abcz(abcz, seed, "xxh64s,mul,sparse")
test_abcz(abcz, seed, "xxh64s,mul,chm,sparse")
test_abcz(abcz, seed, "xxh64s,mul,bdz,sparse")
test_abcz(abcz, seed, "xxh64s,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,chm,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,bdz,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,mod,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,mod,chm,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,mod,bdz,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,mul,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,mul,chm,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,mul,bdz,rank2,sparse")
test_abcz(abcz, seed, "xxh64s,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,chm,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,bdz,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,mod,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,mod,chm,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,mod,bdz,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,mul,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,mul,chm,rank3,sparse")
test_abcz(abcz, seed, "xxh64s,mul,bdz,rank3,sparse")


local function test_build_flags_bad_change(nkeys, flags, build_flags)
	local seed = 0
	local g = assert(rgph.new_graph(nkeys, flags))
	local ok, err = g:build(build_flags, seed, pairs {})
	assert(not ok and err == "invalid value")
end

local function test_build_flags_bad_range(nkeys, flags, build_flags)
	local seed = 0
	local g = assert(rgph.new_graph(nkeys, flags))
	local ok, err = g:build(build_flags, seed, pairs {})
	assert(not ok and err == "out of range")
end

local function test_build_flags_change(keys, seed, flags, build_flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)

	while true do
		local ok, err = g:build(build_flags, seed, pairs(keys))
		if ok then break end
		assert(not err, err) -- pairs(keys) can't fail
		seed = seed + 1
	end

	assert(g:assign() and g:assign(build_flags))
end

local function test_assign_flags_bad_change(keys, seed, flags, assign_flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)

	while true do
		local ok, err = g:build(flags, seed, pairs(keys))
		if ok then break end
		assert(not err, err) -- pairs(keys) can't fail
		seed = seed + 1
	end

	local ok, err = g:assign(assign_flags)
	assert(not ok and err == "invalid value")
end

local function test_assign_flags_change(keys, seed, flags, assign_flags)
	local nkeys = rgph.count_keys(pairs(keys))
	local g = rgph.new_graph(nkeys, flags)

	while true do
		local ok, err = g:build(flags, seed, pairs(keys))
		if ok then break end
		assert(not err, err) -- pairs(keys) can't fail
		seed = seed + 1
	end

	assert(g:assign(assign_flags))
end

test_build_flags_bad_change(1, nil, "rank2")
test_build_flags_bad_change(1, "rank2", "rank3")

test_build_flags_bad_range(rank3_max.s32+1, "jenkins2v", "xxh32s")
test_build_flags_bad_range(rank2_max.s32+1, "jenkins2v,rank2", "xxh32s,rank2")
test_build_flags_bad_range(rank3_max.s32+1, "jenkins2v,rank3", "xxh32s,rank3")

test_build_flags_bad_range(rank3_max.s32+1, "murmur32v", "murmur32s")
test_build_flags_bad_range(rank2_max.s32+1, "murmur32v,rank2", "murmur32s")
test_build_flags_bad_range(rank3_max.s32+1, "murmur32v,rank3", "murmur32s")

test_build_flags_bad_range(rank3_max.s32+1, "xxh64s", "murmur32s")
test_build_flags_bad_range(rank2_max.s32+1, "xxh64s,rank2", "murmur32s,rank2")
test_build_flags_bad_range(rank3_max.s32+1, "xxh64s,rank3", "murmur32s,rank3")

test_build_flags_bad_range(rank3_max.s32+1, "xxh64s", "xxh32s")
test_build_flags_bad_range(rank2_max.s32+1, "xxh64s,rank2", "xxh32s,rank2")
test_build_flags_bad_range(rank3_max.s32+1, "xxh64s,rank3", "xxh32s,rank3")

test_build_flags_change(abcz, seed)
test_build_flags_change(abcz, seed, "", nil)
test_build_flags_change(abcz, seed, nil, "")
test_build_flags_change(abcz, seed, "", "")

-- XXX some tests hang:
test_build_flags_change(abcz, seed, "chm", "bdz")
test_build_flags_change(abcz, seed, "bdz", "chm")
test_build_flags_change(abcz, seed, "mul", "mod")
--test_build_flags_change(abcz, seed, "mod", "mul")
test_build_flags_change(abcz, seed, "sparse", "compact")
test_build_flags_change(abcz, seed, "compact", "sparse")
test_build_flags_change(abcz, seed, "bdz,mul", "chm,mod")
test_build_flags_change(abcz, seed, "chm,mul", "bdz,mod")
--test_build_flags_change(abcz, seed, "bdz,mod", "chm,mul")
--test_build_flags_change(abcz, seed, "chm,mod", "bdz,mul")
test_build_flags_change(abcz, seed, "chm,sparse", "bdz,compact")
test_build_flags_change(abcz, seed, "bdz,sparse", "chm,compact")
test_build_flags_change(abcz, seed, "chm,compact", "bdz,sparse")
test_build_flags_change(abcz, seed, "bdz,compact", "chm,sparse")
test_build_flags_change(abcz, seed, "jenkins2v", "xxh32s")
test_build_flags_change(abcz, seed, "xxh32s", "jenkins2v")
test_build_flags_change(abcz, seed, "murmur32v", "xxh32s")
--test_build_flags_change(abcz, seed, "xxh32s", "murmur32v")
test_build_flags_change(abcz, seed, "jenkins2v", "xxh64s")
test_build_flags_change(abcz, seed, "xxh64s", "jenkins2v")

test_assign_flags_bad_change(abcz, seed, "", "rank2")
test_assign_flags_bad_change(abcz, seed, "rank3", "rank2")
test_assign_flags_bad_change(abcz, seed, "rank2", "rank3")

-- XXX some tests hang:
test_assign_flags_bad_change(abcz, seed, "mod", "mul")
--test_assign_flags_bad_change(abcz, seed, "mul", "mod")
test_assign_flags_bad_change(abcz, seed, "xxh32s", "xxh64s")
--test_assign_flags_bad_change(abcz, seed, "bdz,mul", "chm,mod")
--test_assign_flags_bad_change(abcz, seed, "chm,mul", "bdz,mod")
test_assign_flags_bad_change(abcz, seed, "bdz,mod", "chm,mul")
test_assign_flags_bad_change(abcz, seed, "chm,mod", "bdz,mul")

test_assign_flags_change(abcz, seed, nil, "")
test_assign_flags_change(abcz, seed, "", nil)
test_assign_flags_change(abcz, seed, "", "")

test_assign_flags_change(abcz, seed, "bdz", "chm")
test_assign_flags_change(abcz, seed, "chm", "bdz")
test_assign_flags_change(abcz, seed, nil, "compact")
test_assign_flags_change(abcz, seed, nil, "compact")
test_assign_flags_change(abcz, seed, nil, "sparse")
test_assign_flags_change(abcz, seed, nil, "sparse")
test_assign_flags_change(abcz, seed, "sparse", "compact")
test_assign_flags_change(abcz, seed, "compact", "sparse")
test_assign_flags_change(abcz, seed, "bdz,sparse", "bdz,compact")
test_assign_flags_change(abcz, seed, "bdz,compact", "bdz,sparse")
test_assign_flags_change(abcz, seed, "chm,sparse", "chm,compact")
test_assign_flags_change(abcz, seed, "chm,compact", "chm,sparse")


local zcba = { z=2601, y=2502, x=2403, w=2304, v=2205, u=2106, t=2007,
               s=1908, r=1809, q=1710, p=1611, o=1512, n=1413, m=1314,
	       l=1215, k=1116, j=1017, i=918, h=819, g=720, f=621,
	       e=522, d=423, c=324, b=225, a=126 }

local zero_to_2p31 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, i=8, z=2^31 }
local zero_to_2p32 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, i=8, z=2^32 }
local zero_to_2p33 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, i=8, z=2^33 }
local zero_to_2p51 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, i=8, z=2^51 }
local zero_to_2p52 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, i=8, z=2^52 }
local zero_to_2p53 = { a=0, b=2, c=4, d=6, e=8, f=10, g=12, h=14, z=2^53 }
local zero_to_2p54 = { a=0, b=4, c=8, d=12, e=16, f=20, g=24, h=28, z=2^54 }
local zero_to_2p55 = { a=0, b=8, c=16, d=24, e=32, f=40, g=48, h=56, z=2^55 }
local zero_to_2p56 = { a=0, b=16, c=32, d=48, e=64, f=80, g=96, h=112, z=2^56 }
local zero_to_2p57 = { a=0, b=32, c=64, d=96, e=128, f=160, g=192, z=2^57 }
local zero_to_2p58 = { a=0, b=64, c=128, d=192, e=256, f=320, g=384, z=2^58 }
local zero_to_2p59 = { a=0, b=128, c=256, d=384, e=512, f=640, g=768, z=2^59 }
local zero_to_2p60 = { a=0, b=256, c=512, d=768, e=1024, f=1280, z=2^60 }
local zero_to_2p61 = { a=0, b=512, c=1024, d=1536, e=2048, f=2560, z=2^61 }

local zero_to_2p31m1 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, z=2^31 - 1 }
local zero_to_2p32m1 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, z=2^32 - 1 }
local zero_to_2p33m1 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, z=2^33 - 1 }
local zero_to_2p51m1 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, z=2^51 - 1 }
local zero_to_2p52m1 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, z=2^52 - 1 }
local zero_to_2p53m1 = { a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7, z=2^53 - 1 }

local one_to_2p31 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, z=2^31 }
local one_to_2p32 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, z=2^32 }
local one_to_2p33 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, z=2^33 }
local one_to_2p51 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, z=2^51 }
local one_to_2p52 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9, z=2^52 }
local ulp_to_2p53 = { a=2, b=4, c=6, d=8, e=10, f=12, g=14, h=16, i=18, z=2^53 }
local ulp_to_2p54 = { a=4, b=8, c=12, d=16, e=20, f=24, g=28, h=32, z=2^54 }
local ulp_to_2p55 = { a=8, b=16, c=24, d=32, e=40, f=48, g=56, h=64, z=2^55 }
local ulp_to_2p56 = { a=16, b=32, c=48, d=64, e=80, f=96, g=112, h=128, z=2^56 }
local ulp_to_2p57 = { a=32, b=64, c=96, d=128, e=160, f=192, g=224, z=2^57 }
local ulp_to_2p58 = { a=64, b=128, c=192, d=256, e=320, f=384, g=448, z=2^58 }
local ulp_to_2p59 = { a=128, b=256, c=384, d=512, e=640, f=768, g=896, z=2^59 }
local ulp_to_2p60 = { a=256, b=512, c=768, d=1024, e=1280, f=1536, z=2^60 }
local ulp_to_2p61 = { a=512, b=1024, c=1536, d=2048, e=2560, f=3072, z=2^61 }

local one_to_2p31m1 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, z=2^31 - 1 }
local one_to_2p32m1 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, z=2^32 - 1 }
local one_to_2p33m1 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, z=2^33 - 1 }
local one_to_2p51m1 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, z=2^51 - 1 }
local one_to_2p52m1 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, z=2^52 - 1 }
local one_to_2p53m1 = { a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, z=2^53 - 1 }


-- for k,v,i in index_iter { K=3 } do print(k,v,i) end --> K	nil	3
local function index_iter(keys, v)
	local function iter(t, k)
		local n = next(t, k)
		return n, v, t[n]
	end
	return iter, keys
end

local function minmax(t)
	local min, max
	for k,v in pairs(t) do
		if not min or min > v then
			min = v
		end
		if not max or max < v then
			max = v
		end
	end
	return min, max
end

local function test_index(keys, seed, flags)
	local value = flags and "build with flags " .. flags or "no flags"
	local min,max = minmax(keys)
	local nkeys = rgph.count_keys(index_iter(keys, value))
	local g = rgph.new_graph(nkeys, flags)
	local rank = g:rank()
	local nverts = g:vertices()

	while true do
		local ok, err = g:build(flags, seed, index_iter(keys, value))
		if ok then break end
		assert(not err, err) -- index_iter(keys, value) can't fail
		seed = seed + 1
	end

	assert(g:datalen_min() == #value)
	assert(g:datalen_max() == #value)

	assert(g:index_min() == min)
	assert(g:index_max() == max)

	local assign = assert(g:assign("chm"))

	local compact = g:index() == "compact"
	local unassigned = compact and max - min + 1 or align_up_pow2(max)

	assert(unassigned > 0, "unassigned wrapped around at 2^63")
	assert(unassigned > max - min, "unassigned is rounded down")

	assert(g:index_min() == min)
	assert(g:index_max() == max)

	for v = 0, nverts - 1 do
		assert(assign[v] >= 0 and assign[v] < unassigned)
	end

	local e = 0
	for k, _, i in index_iter(keys) do
		e = e + 1
		local edges = { g:edge(e) }

		local h = 0
		for j = 1, rank do
			local v = edges[j]
			local a = assign[v]
			assert(a < unassigned, "must be assigned")
			h = h - unassigned + a -- Avoid abs values > unassigned.
			if h < 0 then h = h + unassigned end
		end

		if compact then
			h = h + min
		end

		assert(h == i, "must be order-preserving minimal hash")
	end
end

test_index(abcz, seed, "bdz,rank2")
test_index(abcz, seed, "bdz,rank3")
test_index(abcz, seed, "chm,rank2")
test_index(abcz, seed, "chm,rank3")
test_index(abcz, seed, "bdz,rank2,sparse")
test_index(abcz, seed, "bdz,rank3,sparse")
test_index(abcz, seed, "chm,rank2,sparse")
test_index(abcz, seed, "chm,rank3,sparse")
test_index(abcz, seed, "bdz,rank2,compact")
test_index(abcz, seed, "bdz,rank3,compact")
test_index(abcz, seed, "chm,rank2,compact")
test_index(abcz, seed, "chm,rank3,compact")

test_index(zcba, seed, "bdz,rank2")
test_index(zcba, seed, "bdz,rank3")
test_index(zcba, seed, "chm,rank2")
test_index(zcba, seed, "chm,rank3")
test_index(zcba, seed, "bdz,rank2,sparse")
test_index(zcba, seed, "bdz,rank3,sparse")
test_index(zcba, seed, "chm,rank2,sparse")
test_index(zcba, seed, "chm,rank3,sparse")
test_index(zcba, seed, "bdz,rank2,compact")
test_index(zcba, seed, "bdz,rank3,compact")
test_index(zcba, seed, "chm,rank2,compact")
test_index(zcba, seed, "chm,rank3,compact")

test_index(zero_to_2p31, seed, "bdz,rank2")
test_index(zero_to_2p31, seed, "bdz,rank3")
test_index(zero_to_2p31, seed, "chm,rank2")
test_index(zero_to_2p31, seed, "chm,rank3")
test_index(zero_to_2p31, seed, "bdz,rank2,sparse")
test_index(zero_to_2p31, seed, "bdz,rank3,sparse")
test_index(zero_to_2p31, seed, "chm,rank2,sparse")
test_index(zero_to_2p31, seed, "chm,rank3,sparse")
test_index(zero_to_2p31, seed, "bdz,rank2,compact")
test_index(zero_to_2p31, seed, "bdz,rank3,compact")
test_index(zero_to_2p31, seed, "chm,rank2,compact")
test_index(zero_to_2p31, seed, "chm,rank3,compact")

test_index(zero_to_2p32, seed, "bdz,rank2")
test_index(zero_to_2p32, seed, "bdz,rank3")
test_index(zero_to_2p32, seed, "chm,rank2")
test_index(zero_to_2p32, seed, "chm,rank3")
test_index(zero_to_2p32, seed, "bdz,rank2,sparse")
test_index(zero_to_2p32, seed, "bdz,rank3,sparse")
test_index(zero_to_2p32, seed, "chm,rank2,sparse")
test_index(zero_to_2p32, seed, "chm,rank3,sparse")
test_index(zero_to_2p32, seed, "bdz,rank2,compact")
test_index(zero_to_2p32, seed, "bdz,rank3,compact")
test_index(zero_to_2p32, seed, "chm,rank2,compact")
test_index(zero_to_2p32, seed, "chm,rank3,compact")

test_index(zero_to_2p33, seed, "bdz,rank2")
test_index(zero_to_2p33, seed, "bdz,rank3")
test_index(zero_to_2p33, seed, "chm,rank2")
test_index(zero_to_2p33, seed, "chm,rank3")
test_index(zero_to_2p33, seed, "bdz,rank2,sparse")
test_index(zero_to_2p33, seed, "bdz,rank3,sparse")
test_index(zero_to_2p33, seed, "chm,rank2,sparse")
test_index(zero_to_2p33, seed, "chm,rank3,sparse")
test_index(zero_to_2p33, seed, "bdz,rank2,compact")
test_index(zero_to_2p33, seed, "bdz,rank3,compact")
test_index(zero_to_2p33, seed, "chm,rank2,compact")
test_index(zero_to_2p33, seed, "chm,rank3,compact")

test_index(zero_to_2p51, seed, "bdz,rank2")
test_index(zero_to_2p51, seed, "bdz,rank3")
test_index(zero_to_2p51, seed, "chm,rank2")
test_index(zero_to_2p51, seed, "chm,rank3")
test_index(zero_to_2p51, seed, "bdz,rank2,sparse")
test_index(zero_to_2p51, seed, "bdz,rank3,sparse")
test_index(zero_to_2p51, seed, "chm,rank2,sparse")
test_index(zero_to_2p51, seed, "chm,rank3,sparse")
test_index(zero_to_2p51, seed, "bdz,rank2,compact")
test_index(zero_to_2p51, seed, "bdz,rank3,compact")
test_index(zero_to_2p51, seed, "chm,rank2,compact")
test_index(zero_to_2p51, seed, "chm,rank3,compact")

test_index(zero_to_2p52, seed, "bdz,rank2")
test_index(zero_to_2p52, seed, "bdz,rank3")
test_index(zero_to_2p52, seed, "chm,rank2")
test_index(zero_to_2p52, seed, "chm,rank3")
test_index(zero_to_2p52, seed, "bdz,rank2,sparse")
test_index(zero_to_2p52, seed, "bdz,rank3,sparse")
test_index(zero_to_2p52, seed, "chm,rank2,sparse")
test_index(zero_to_2p52, seed, "chm,rank3,sparse")
test_index(zero_to_2p52, seed, "bdz,rank2,compact")
test_index(zero_to_2p52, seed, "bdz,rank3,compact")
test_index(zero_to_2p52, seed, "chm,rank2,compact")
test_index(zero_to_2p52, seed, "chm,rank3,compact")

test_index(zero_to_2p53, seed, "bdz,rank2,sparse")
test_index(zero_to_2p53, seed, "bdz,rank3,sparse")
test_index(zero_to_2p53, seed, "chm,rank2,sparse")
test_index(zero_to_2p53, seed, "chm,rank3,sparse")

test_index(zero_to_2p54, seed, "bdz,rank2,sparse")
test_index(zero_to_2p54, seed, "bdz,rank3,sparse")
test_index(zero_to_2p54, seed, "chm,rank2,sparse")
test_index(zero_to_2p54, seed, "chm,rank3,sparse")

test_index(zero_to_2p55, seed, "bdz,rank2,sparse")
test_index(zero_to_2p55, seed, "bdz,rank3,sparse")
test_index(zero_to_2p55, seed, "chm,rank2,sparse")
test_index(zero_to_2p55, seed, "chm,rank3,sparse")

test_index(zero_to_2p56, seed, "bdz,rank2,sparse")
test_index(zero_to_2p56, seed, "bdz,rank3,sparse")
test_index(zero_to_2p56, seed, "chm,rank2,sparse")
test_index(zero_to_2p56, seed, "chm,rank3,sparse")

test_index(zero_to_2p57, seed, "bdz,rank2,sparse")
test_index(zero_to_2p57, seed, "bdz,rank3,sparse")
test_index(zero_to_2p57, seed, "chm,rank2,sparse")
test_index(zero_to_2p57, seed, "chm,rank3,sparse")

test_index(zero_to_2p58, seed, "bdz,rank2,sparse")
test_index(zero_to_2p58, seed, "bdz,rank3,sparse")
test_index(zero_to_2p58, seed, "chm,rank2,sparse")
test_index(zero_to_2p58, seed, "chm,rank3,sparse")

test_index(zero_to_2p59, seed, "bdz,rank2,sparse")
test_index(zero_to_2p59, seed, "bdz,rank3,sparse")
test_index(zero_to_2p59, seed, "chm,rank2,sparse")
test_index(zero_to_2p59, seed, "chm,rank3,sparse")

test_index(zero_to_2p60, seed, "bdz,rank2,sparse")
test_index(zero_to_2p60, seed, "bdz,rank3,sparse")
test_index(zero_to_2p60, seed, "chm,rank2,sparse")
test_index(zero_to_2p60, seed, "chm,rank3,sparse")

test_index(zero_to_2p61, seed, "bdz,rank2,sparse")
test_index(zero_to_2p61, seed, "bdz,rank3,sparse")
test_index(zero_to_2p61, seed, "chm,rank2,sparse")
test_index(zero_to_2p61, seed, "chm,rank3,sparse")

test_index(zero_to_2p31m1, seed, "bdz,rank2")
test_index(zero_to_2p31m1, seed, "bdz,rank3")
test_index(zero_to_2p31m1, seed, "chm,rank2")
test_index(zero_to_2p31m1, seed, "chm,rank3")
test_index(zero_to_2p31m1, seed, "bdz,rank2,sparse")
test_index(zero_to_2p31m1, seed, "bdz,rank3,sparse")
test_index(zero_to_2p31m1, seed, "chm,rank2,sparse")
test_index(zero_to_2p31m1, seed, "chm,rank3,sparse")
test_index(zero_to_2p31m1, seed, "bdz,rank2,compact")
test_index(zero_to_2p31m1, seed, "bdz,rank3,compact")
test_index(zero_to_2p31m1, seed, "chm,rank2,compact")
test_index(zero_to_2p31m1, seed, "chm,rank3,compact")

test_index(zero_to_2p32m1, seed, "bdz,rank2")
test_index(zero_to_2p32m1, seed, "bdz,rank3")
test_index(zero_to_2p32m1, seed, "chm,rank2")
test_index(zero_to_2p32m1, seed, "chm,rank3")
test_index(zero_to_2p32m1, seed, "bdz,rank2,sparse")
test_index(zero_to_2p32m1, seed, "bdz,rank3,sparse")
test_index(zero_to_2p32m1, seed, "chm,rank2,sparse")
test_index(zero_to_2p32m1, seed, "chm,rank3,sparse")
test_index(zero_to_2p32m1, seed, "bdz,rank2,compact")
test_index(zero_to_2p32m1, seed, "bdz,rank3,compact")
test_index(zero_to_2p32m1, seed, "chm,rank2,compact")
test_index(zero_to_2p32m1, seed, "chm,rank3,compact")

test_index(zero_to_2p33m1, seed, "bdz,rank2")
test_index(zero_to_2p33m1, seed, "bdz,rank3")
test_index(zero_to_2p33m1, seed, "chm,rank2")
test_index(zero_to_2p33m1, seed, "chm,rank3")
test_index(zero_to_2p33m1, seed, "bdz,rank2,sparse")
test_index(zero_to_2p33m1, seed, "bdz,rank3,sparse")
test_index(zero_to_2p33m1, seed, "chm,rank2,sparse")
test_index(zero_to_2p33m1, seed, "chm,rank3,sparse")
test_index(zero_to_2p33m1, seed, "bdz,rank2,compact")
test_index(zero_to_2p33m1, seed, "bdz,rank3,compact")
test_index(zero_to_2p33m1, seed, "chm,rank2,compact")
test_index(zero_to_2p33m1, seed, "chm,rank3,compact")

test_index(zero_to_2p51m1, seed, "bdz,rank2")
test_index(zero_to_2p51m1, seed, "bdz,rank3")
test_index(zero_to_2p51m1, seed, "chm,rank2")
test_index(zero_to_2p51m1, seed, "chm,rank3")
test_index(zero_to_2p51m1, seed, "bdz,rank2,sparse")
test_index(zero_to_2p51m1, seed, "bdz,rank3,sparse")
test_index(zero_to_2p51m1, seed, "chm,rank2,sparse")
test_index(zero_to_2p51m1, seed, "chm,rank3,sparse")
test_index(zero_to_2p51m1, seed, "bdz,rank2,compact")
test_index(zero_to_2p51m1, seed, "bdz,rank3,compact")
test_index(zero_to_2p51m1, seed, "chm,rank2,compact")
test_index(zero_to_2p51m1, seed, "chm,rank3,compact")

test_index(zero_to_2p52m1, seed, "bdz,rank2")
test_index(zero_to_2p52m1, seed, "bdz,rank3")
test_index(zero_to_2p52m1, seed, "chm,rank2")
test_index(zero_to_2p52m1, seed, "chm,rank3")
test_index(zero_to_2p52m1, seed, "bdz,rank2,sparse")
test_index(zero_to_2p52m1, seed, "bdz,rank3,sparse")
test_index(zero_to_2p52m1, seed, "chm,rank2,sparse")
test_index(zero_to_2p52m1, seed, "chm,rank3,sparse")
test_index(zero_to_2p52m1, seed, "bdz,rank2,compact")
test_index(zero_to_2p52m1, seed, "bdz,rank3,compact")
test_index(zero_to_2p52m1, seed, "chm,rank2,compact")
test_index(zero_to_2p52m1, seed, "chm,rank3,compact")

test_index(zero_to_2p53m1, seed, "bdz,rank2")
test_index(zero_to_2p53m1, seed, "bdz,rank3")
test_index(zero_to_2p53m1, seed, "chm,rank2")
test_index(zero_to_2p53m1, seed, "chm,rank3")
test_index(zero_to_2p53m1, seed, "bdz,rank2,sparse")
test_index(zero_to_2p53m1, seed, "bdz,rank3,sparse")
test_index(zero_to_2p52m1, seed, "chm,rank2,sparse")
test_index(zero_to_2p52m1, seed, "chm,rank3,sparse")
test_index(zero_to_2p53m1, seed, "bdz,rank2,compact")
test_index(zero_to_2p53m1, seed, "bdz,rank3,compact")
test_index(zero_to_2p53m1, seed, "chm,rank2,compact")
test_index(zero_to_2p53m1, seed, "chm,rank3,compact")

test_index(one_to_2p31, seed, "bdz,rank2")
test_index(one_to_2p31, seed, "bdz,rank3")
test_index(one_to_2p31, seed, "chm,rank2")
test_index(one_to_2p31, seed, "chm,rank3")
test_index(one_to_2p31, seed, "bdz,rank2,sparse")
test_index(one_to_2p31, seed, "bdz,rank3,sparse")
test_index(one_to_2p31, seed, "chm,rank2,sparse")
test_index(one_to_2p31, seed, "chm,rank3,sparse")
test_index(one_to_2p31, seed, "bdz,rank2,compact")
test_index(one_to_2p31, seed, "bdz,rank3,compact")
test_index(one_to_2p31, seed, "chm,rank2,compact")
test_index(one_to_2p31, seed, "chm,rank3,compact")

test_index(one_to_2p32, seed, "bdz,rank2")
test_index(one_to_2p32, seed, "bdz,rank3")
test_index(one_to_2p32, seed, "chm,rank2")
test_index(one_to_2p32, seed, "chm,rank3")
test_index(one_to_2p32, seed, "bdz,rank2,sparse")
test_index(one_to_2p32, seed, "bdz,rank3,sparse")
test_index(one_to_2p32, seed, "chm,rank2,sparse")
test_index(one_to_2p32, seed, "chm,rank3,sparse")
test_index(one_to_2p32, seed, "bdz,rank2,compact")
test_index(one_to_2p32, seed, "bdz,rank3,compact")
test_index(one_to_2p32, seed, "chm,rank2,compact")
test_index(one_to_2p32, seed, "chm,rank3,compact")

test_index(one_to_2p33, seed, "bdz,rank2")
test_index(one_to_2p33, seed, "bdz,rank3")
test_index(one_to_2p33, seed, "chm,rank2")
test_index(one_to_2p33, seed, "chm,rank3")
test_index(one_to_2p33, seed, "bdz,rank2,sparse")
test_index(one_to_2p33, seed, "bdz,rank3,sparse")
test_index(one_to_2p33, seed, "chm,rank2,sparse")
test_index(one_to_2p33, seed, "chm,rank3,sparse")
test_index(one_to_2p33, seed, "bdz,rank2,compact")
test_index(one_to_2p33, seed, "bdz,rank3,compact")
test_index(one_to_2p33, seed, "chm,rank2,compact")
test_index(one_to_2p33, seed, "chm,rank3,compact")

test_index(one_to_2p51, seed, "bdz,rank2")
test_index(one_to_2p51, seed, "bdz,rank3")
test_index(one_to_2p51, seed, "chm,rank2")
test_index(one_to_2p51, seed, "chm,rank3")
test_index(one_to_2p51, seed, "bdz,rank2,sparse")
test_index(one_to_2p51, seed, "bdz,rank3,sparse")
test_index(one_to_2p51, seed, "chm,rank2,sparse")
test_index(one_to_2p51, seed, "chm,rank3,sparse")
test_index(one_to_2p51, seed, "bdz,rank2,compact")
test_index(one_to_2p51, seed, "bdz,rank3,compact")
test_index(one_to_2p51, seed, "chm,rank2,compact")
test_index(one_to_2p51, seed, "chm,rank3,compact")

test_index(one_to_2p52, seed, "bdz,rank2")
test_index(one_to_2p52, seed, "bdz,rank3")
test_index(one_to_2p52, seed, "chm,rank2")
test_index(one_to_2p52, seed, "chm,rank3")
test_index(one_to_2p52, seed, "bdz,rank2,sparse")
test_index(one_to_2p52, seed, "bdz,rank3,sparse")
test_index(one_to_2p52, seed, "chm,rank2,sparse")
test_index(one_to_2p52, seed, "chm,rank3,sparse")
test_index(one_to_2p52, seed, "bdz,rank2,compact")
test_index(one_to_2p52, seed, "bdz,rank3,compact")
test_index(one_to_2p52, seed, "chm,rank2,compact")
test_index(one_to_2p52, seed, "chm,rank3,compact")

test_index(ulp_to_2p53, seed, "bdz,rank2")
test_index(ulp_to_2p53, seed, "bdz,rank3")
test_index(ulp_to_2p53, seed, "chm,rank2")
test_index(ulp_to_2p53, seed, "chm,rank3")
test_index(ulp_to_2p53, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p53, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p53, seed, "chm,rank2,sparse")
test_index(ulp_to_2p53, seed, "chm,rank3,sparse")
test_index(ulp_to_2p53, seed, "bdz,rank2,compact")
test_index(ulp_to_2p53, seed, "bdz,rank3,compact")
test_index(ulp_to_2p53, seed, "chm,rank2,compact")
test_index(ulp_to_2p53, seed, "chm,rank3,compact")

test_index(ulp_to_2p54, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p54, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p54, seed, "chm,rank2,sparse")
test_index(ulp_to_2p54, seed, "chm,rank3,sparse")

test_index(ulp_to_2p55, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p55, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p55, seed, "chm,rank2,sparse")
test_index(ulp_to_2p55, seed, "chm,rank3,sparse")

test_index(ulp_to_2p56, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p56, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p56, seed, "chm,rank2,sparse")
test_index(ulp_to_2p56, seed, "chm,rank3,sparse")

test_index(ulp_to_2p57, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p57, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p57, seed, "chm,rank2,sparse")
test_index(ulp_to_2p57, seed, "chm,rank3,sparse")

test_index(ulp_to_2p58, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p58, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p58, seed, "chm,rank2,sparse")
test_index(ulp_to_2p58, seed, "chm,rank3,sparse")

test_index(ulp_to_2p59, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p59, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p59, seed, "chm,rank2,sparse")
test_index(ulp_to_2p59, seed, "chm,rank3,sparse")

test_index(ulp_to_2p60, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p60, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p60, seed, "chm,rank2,sparse")
test_index(ulp_to_2p60, seed, "chm,rank3,sparse")

test_index(ulp_to_2p61, seed, "bdz,rank2,sparse")
test_index(ulp_to_2p61, seed, "bdz,rank3,sparse")
test_index(ulp_to_2p61, seed, "chm,rank2,sparse")
test_index(ulp_to_2p61, seed, "chm,rank3,sparse")

test_index(one_to_2p31m1, seed, "bdz,rank2")
test_index(one_to_2p31m1, seed, "bdz,rank3")
test_index(one_to_2p31m1, seed, "chm,rank2")
test_index(one_to_2p31m1, seed, "chm,rank3")
test_index(one_to_2p31m1, seed, "bdz,rank2,sparse")
test_index(one_to_2p31m1, seed, "bdz,rank3,sparse")
test_index(one_to_2p31m1, seed, "chm,rank2,sparse")
test_index(one_to_2p31m1, seed, "chm,rank3,sparse")
test_index(one_to_2p31m1, seed, "bdz,rank2,compact")
test_index(one_to_2p31m1, seed, "bdz,rank3,compact")
test_index(one_to_2p31m1, seed, "chm,rank2,compact")
test_index(one_to_2p31m1, seed, "chm,rank3,compact")

test_index(one_to_2p32m1, seed, "bdz,rank2")
test_index(one_to_2p32m1, seed, "bdz,rank3")
test_index(one_to_2p32m1, seed, "chm,rank2")
test_index(one_to_2p32m1, seed, "chm,rank3")
test_index(one_to_2p32m1, seed, "bdz,rank2,sparse")
test_index(one_to_2p32m1, seed, "bdz,rank3,sparse")
test_index(one_to_2p32m1, seed, "chm,rank2,sparse")
test_index(one_to_2p32m1, seed, "chm,rank3,sparse")
test_index(one_to_2p32m1, seed, "bdz,rank2,compact")
test_index(one_to_2p32m1, seed, "bdz,rank3,compact")
test_index(one_to_2p32m1, seed, "chm,rank2,compact")
test_index(one_to_2p32m1, seed, "chm,rank3,compact")

test_index(one_to_2p33m1, seed, "bdz,rank2")
test_index(one_to_2p33m1, seed, "bdz,rank3")
test_index(one_to_2p33m1, seed, "chm,rank2")
test_index(one_to_2p33m1, seed, "chm,rank3")
test_index(one_to_2p33m1, seed, "bdz,rank2,sparse")
test_index(one_to_2p33m1, seed, "bdz,rank3,sparse")
test_index(one_to_2p33m1, seed, "chm,rank2,sparse")
test_index(one_to_2p33m1, seed, "chm,rank3,sparse")
test_index(one_to_2p33m1, seed, "bdz,rank2,compact")
test_index(one_to_2p33m1, seed, "bdz,rank3,compact")
test_index(one_to_2p33m1, seed, "chm,rank2,compact")
test_index(one_to_2p33m1, seed, "chm,rank3,compact")

test_index(one_to_2p51m1, seed, "bdz,rank2")
test_index(one_to_2p51m1, seed, "bdz,rank3")
test_index(one_to_2p51m1, seed, "chm,rank2")
test_index(one_to_2p51m1, seed, "chm,rank3")
test_index(one_to_2p51m1, seed, "bdz,rank2,sparse")
test_index(one_to_2p51m1, seed, "bdz,rank3,sparse")
test_index(one_to_2p51m1, seed, "chm,rank2,sparse")
test_index(one_to_2p51m1, seed, "chm,rank3,sparse")
test_index(one_to_2p51m1, seed, "bdz,rank2,compact")
test_index(one_to_2p51m1, seed, "bdz,rank3,compact")
test_index(one_to_2p51m1, seed, "chm,rank2,compact")
test_index(one_to_2p51m1, seed, "chm,rank3,compact")

test_index(one_to_2p52m1, seed, "bdz,rank2")
test_index(one_to_2p52m1, seed, "bdz,rank3")
test_index(one_to_2p52m1, seed, "chm,rank2")
test_index(one_to_2p52m1, seed, "chm,rank3")
test_index(one_to_2p52m1, seed, "bdz,rank2,sparse")
test_index(one_to_2p52m1, seed, "bdz,rank3,sparse")
test_index(one_to_2p52m1, seed, "chm,rank2,sparse")
test_index(one_to_2p52m1, seed, "chm,rank3,sparse")
test_index(one_to_2p52m1, seed, "bdz,rank2,compact")
test_index(one_to_2p52m1, seed, "bdz,rank3,compact")
test_index(one_to_2p52m1, seed, "chm,rank2,compact")
test_index(one_to_2p52m1, seed, "chm,rank3,compact")

test_index(one_to_2p53m1, seed, "bdz,rank2")
test_index(one_to_2p53m1, seed, "bdz,rank3")
test_index(one_to_2p53m1, seed, "chm,rank2")
test_index(one_to_2p53m1, seed, "chm,rank3")
test_index(one_to_2p53m1, seed, "bdz,rank2,sparse")
test_index(one_to_2p53m1, seed, "bdz,rank3,sparse")
test_index(one_to_2p52m1, seed, "chm,rank2,sparse")
test_index(one_to_2p52m1, seed, "chm,rank3,sparse")
test_index(one_to_2p53m1, seed, "bdz,rank2,compact")
test_index(one_to_2p53m1, seed, "bdz,rank3,compact")
test_index(one_to_2p53m1, seed, "chm,rank2,compact")
test_index(one_to_2p53m1, seed, "chm,rank3,compact")


local function has_idiv()
	local ok, fn = pcall(load, "return 1//1")
	return ok and fn ~= nil
end

-- Test that Lua 5.3 module doesn't round big numbers.
if has_idiv() then
	local big61 = {
		a=1, b=3, c=13, d=137,
		E = 2305843009213693815, -- 2^61-137
		F = 2305843009213693939, -- 2^61-13
		G = 2305843009213693949, -- 2^61-3
	}

	local big62 = {
		a=1, b=3, c=13, d=137,
		E = 4611686018427387767, -- 2^62-137
		F = 4611686018427387891, -- 2^62-13
		G = 4611686018427387901, -- 2^62-3
	}

	local big63 = {
		a=1, b=3, c=13, d=137,
		E = 9223372036854775671, -- 2^63-137
		F = 9223372036854775795, -- 2^63-13
		G = 9223372036854775805, -- 2^63-3
	}

	assert(big61.E ~= big61.E / 1)
	assert(big61.F ~= big61.F / 1)
	assert(big61.G ~= big61.G / 1)

	assert(big62.E ~= big62.E / 1)
	assert(big62.F ~= big62.F / 1)
	assert(big62.G ~= big62.G / 1)

	assert(big63.E ~= big63.E / 1)
	assert(big63.F ~= big63.F / 1)
	assert(big63.G ~= big63.G / 1)

	test_index(big61, seed, "bdz,rank2")
	test_index(big61, seed, "bdz,rank3")
	test_index(big61, seed, "chm,rank2")
	test_index(big61, seed, "chm,rank3")
	test_index(big61, seed, "bdz,rank2,sparse")
	test_index(big61, seed, "bdz,rank3,sparse")
	test_index(big61, seed, "chm,rank2,sparse")
	test_index(big61, seed, "chm,rank3,sparse")
	test_index(big61, seed, "bdz,rank2,compact")
	test_index(big61, seed, "bdz,rank3,compact")
	test_index(big61, seed, "chm,rank2,compact")
	test_index(big61, seed, "chm,rank3,compact")

	test_index(big62, seed, "bdz,rank2,compact")
	test_index(big62, seed, "bdz,rank3,compact")
	test_index(big62, seed, "chm,rank2,compact")
	test_index(big62, seed, "chm,rank3,compact")

	test_index(big63, seed, "bdz,rank2,compact")
	test_index(big63, seed, "bdz,rank3,compact")
	test_index(big63, seed, "chm,rank2,compact")
	test_index(big63, seed, "chm,rank3,compact")
end
