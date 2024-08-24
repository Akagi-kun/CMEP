local config = require("config")

local calculateMapOffsetY_impl = function(y)
	return ((y) * config.chunk_size_x * config.chunk_size_z)
end

validateX_impl = function(val)
    return val ~= nil and 0 < val and val < (config.chunk_size_x + 1)
end
validateY_impl = function(val)
    return val ~= nil and 0 < val and val < (config.chunk_size_y + 1)
end
validateZ_impl = function(val)
    return val ~= nil and 0 < val and val < (config.chunk_size_z + 1)
end

return {
    validateX = validateX_impl,
    validateY = validateY_impl,
    validateZ = validateZ_impl,

    calculateMapOffsetY = calculateMapOffsetY_impl,
    calculateMapOffset = function(x, y, z)
    	assert(validateX_impl(x) and validateY_impl(y) and validateZ_impl(z), string.format("Failed assert at calculateMapOffset with params %f %f %f", x, y, z))
    	return (x - 1) + calculateMapOffsetY_impl(y - 1) + ((z - 1) * config.chunk_size_x)
    end,
}