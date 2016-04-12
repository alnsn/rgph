local dasm   = require "dasm"
local dynasm = require "dynasm" -- to load dynasm_hash_x86.dasl file
local hash   = require "dynasm_hash_x86"
local pretty = require "pl.pretty"

local args = { ... }
local ntries = 20
local seed = tonumber(args[1] or 0xbb8bb8d2) -- 10727
local keys = {
	["\0\0\0\0"] = "zero",
	["\1\0\0\0"] = "one",
	["\2\0\0\0"] = "two",
	["\3\0\0\0"] = "three",
	["\4\0\0\0"] = "four",
	["\5\0\0\0"] = "five",
	["\6\0\0\0"] = "six",
	["\7\0\0\0"] = "seven",
	["\8\0\0\0"] = "eight",
	["\9\0\0\0"] = "nine"
}

local code, buf, size = hash.generate(keys,
    "xxh32s,bdz,rank2,pow2", ntries, seed)
dasm.dump(buf, size)

local hashes = {}
local hash_hits = {}
for key,_ in pairs(keys) do
	local i = string.byte(key:sub(1,1))
	local h = code(i)
	if hash_hits[h] then
		error(string.format("duplicate hash, seed=%d", seed))
	end
	hash_hits[h] = true
	table.insert(hashes, h)
end

table.sort(hashes)
pretty.dump(hashes)
