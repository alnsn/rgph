local dasm   = require "dasm"
local dynasm = require "dynasm" -- to load dynasm_hash.dasl file
local hash   = require "dynasm_hash"
local pretty = require "pl.pretty"

local args = { ... }
local ntries = 20
local seed = tonumber(args[1] or 0xbb8bb8d2)
local keys = {
	-- FourCC codes
	AIFF = "unused",
	DIVX = "unused",
	GEOX = "unused",
	H264 = "unused",
	I420 = "unused",
	MRLE = "unused",
	MSVC = "unused",
	RIFF = "unused",
	SVID = "unused",
	XDIV = "unused"
}

local code, buf, size = hash.generate(keys,
    "xxh32s,bdz,rank3,fastdiv", ntries, seed)
dasm.dump(buf, size)

local function fourCC(str)
	local a,b,c,d = str:byte(1, 4)
	return d * 16777216 + c * 65536 + b * 256 + a
end

local hashes = {}
local hash_hits = {}
for key,_ in pairs(keys) do
	local i = fourCC(key)
	local h = code(i)
	if hash_hits[h] then
		error(string.format("duplicate hash, seed=%d", seed))
	end
	hash_hits[h] = true
	table.insert(hashes, h)
end

table.sort(hashes)
pretty.dump(hashes)
