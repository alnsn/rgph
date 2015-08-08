local rgph = require "rgph"

local function dot(file, g, ...)
	local iter = select(1, ...) ~= nil

	file:write("graph {\n")
	for a,b,c in g:edges(...) do
		local h1 = iter and b or a
		local h2 = iter and c or b
		local s  = iter and a or ""

		file:write(string.format("%d -- %d [label=%q];\n", h1, h2, s))
	end
	local label = string.format("%d edges, %d vertices, seed %d",
	    g:entries(), g:vertices(), g:seed())
	file:write(string.format("label=%q;}\n", label))
end

return dot
