local rgph = require "rgph"

local function dot(file, g, ...)
	local iter = select(1, ...) ~= nil

	file:write("graph {\n")
	for h1, h2, peel, key in g:edges("peel", ...) do
		local color = peel > 0 and "black" or "red"
		local label = string.format("%d/%s", peel, key)
		file:write(string.format("%d -- %d [label=%q, color=%q];\n",
		    h1, h2, label, color))
	end
	local label = string.format("%d edges, %d vertices, seed %d",
	    g:entries(), g:vertices(), g:seed())
	file:write(string.format("label=%q;}\n", label))
end

return dot
