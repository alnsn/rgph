local dasm   = require "dasm"
local dynasm = require "dynasm" -- to load dynasm_hash.dasl file
local hash   = require "dynasm_hash"
local pretty = require "pl.pretty"

local args = { ... }
local ntries = 20
local seed = tonumber(args[1] or 0xbb8bb8d2)
local keys = {
	-- FourCC codes
	AIFF = 10,
	DIVX = 1000,
	GEOX = 2000,
	H264 = 3000,
	I420 = 4000, -- 0,
	MRLE = 5000, -- 1,
	MSVC = 6000, -- 2,
	RIFF = 7000, -- 3,
	SVID = 8000, -- 0,
	XDIV = 9000, -- 1,
}

local code, buf, size = hash.generate(keys,
    "xxh32s,chm,rank2,fastdiv", ntries, seed)

local function fourCC(str)
	local a,b,c,d = str:byte(1, 4)
	return d * 16777216 + c * 65536 + b * 256 + a
end

if true then
	dasm.dump(buf, size)

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
else
	for i = 1, 1e8 do
		code(i)
	end
end
