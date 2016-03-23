local rgph = require "rgph"

local function dot(file, g, ...)
	local iter = select(1, ...) ~= nil

	file:write("graph {\n")
	local edge_fmt = "%d -- %d [label=%q, color=%q];\n"
	for peel, key, h1, h2, h3 in g:edges("peel,key", ...) do
		local color = peel > 0 and "black" or "red"
		local label = string.format("%s", key)
		file:write(edge_fmt:format(h1, h2, label, color))
		if h3 then
			file:write(edge_fmt:format(h2, h3, label, color))
		end
	end
	local label_fmt = "%d edges, %d vertices, core %d, seed %d, flags %q"
	local label = label_fmt:format(g:entries(),
	    g:vertices(), g:core_size(), g:seed(), g:flags())
	file:write(string.format("label=%q;}\n", label))
end

return { dot=dot }
