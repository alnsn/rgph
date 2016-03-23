local rgph = require "rgph"

local edge_fmt = "%d -- %d [label=%q, color=%q];\n"
local graph_label_fmt = "%d edges, %d vertices, core %d, seed %d, flags %q"

local function dot(file, g, ...)
	local seed = g:seed()
	--local hash = rgph[g:hash()]

	file:write("graph {\n")

	for peel, key, h1, h2, h3 in g:edges("peel,key", ...) do
		local color = peel > 0 and "black" or "red"
		local label = string.format("%s", key)
	--	local label = string.format("%s %x %x %x", key, hash(key, seed))
		file:write(edge_fmt:format(h1, h2, label, color))
		if h3 then
			file:write(edge_fmt:format(h2, h3, label, color))
		end
	end

	local label = graph_label_fmt:format(g:entries(),
	    g:vertices(), g:core_size(), seed, g:flags())
	file:write(string.format("label=%q;}\n", label))
end

return { dot=dot }
