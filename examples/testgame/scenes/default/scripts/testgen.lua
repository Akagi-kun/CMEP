local dynagen_defs = require("dynagen_defs")
local config = require("config")
local gamedefs = require("game_defs")
--require("perlin")

--[[ local tracemap = {}
function trace (event)
	local info = debug.getinfo(2)
	local source = info.source
	local func_name = string.format("(%s)%s", info.what, info.name)
	local line = info.linedefined

	if tracemap[source] == nil then tracemap[source] = {} end
	local tracemap_local = tracemap[source]

	if tracemap_local[line] == nil then
		tracemap_local[line] = {1, func_name}
	else
		tracemap_local[line][1] = tracemap_local[line][1] + 1
	end
	--local s = debug.getinfo(2, "S").short_src
	--print(s .. ":" .. line)
end

function print_trace()
	print("Starting trace...")
	for key, table in pairs(tracemap) do
		for i, val in pairs(table) do
			if val ~= nil then
				print(string.format("%s:%i fn %s %u", key, i, val[2], val[1]))
			end
		end
	end
end

debug.sethook(trace, "lc", 1) ]]

local map = {}

local calculateMapOffsetZ = function(z)
	return (config.chunk_size_x * z)
end

local calculateMapOffset = function(x, y, z)
	return x + (calculateMapOffsetZ(z - 1)) + (calculateMapOffsetZ(config.chunk_size_z)) * (y - 1)
end

--[[ local generateTree = function(x, y, z)
	local leaves = gamedefs.block_types.LEAVES
	local wood = gamedefs.block_types.WOOD
	local tree_def = {
		{
			0, 0, 0, 	0, 0,
			0, 0, 0, 	0, 0,
			0, 0, wood, 0, 0,
			0, 0, 0, 	0, 0,
			0, 0, 0, 	0, 0,
		},
		{
			0, 0, 0, 	0, 0,
			0, 0, 0, 	0, 0,
			0, 0, wood, 0, 0,
			0, 0, 0, 	0, 0,
			0, 0, 0, 	0, 0,
		},
		{
			0, 0, 0, 	0, 0,
			0, 0, 0, 	0, 0,
			0, 0, wood, 0, 0,
			0, 0, 0, 	0, 0,
			0, 0, 0, 	0, 0,
		},
		{
			0, 5, 5, 	5, 0,
			5, 5, 5, 	5, 5,
			5, 5, wood, 5, 5,
			5, 5, 5, 	5, 5,
			0, 5, 5, 	5, 0,
		},
		{
			0, 		leaves, leaves, leaves, 0,
			leaves, leaves, leaves, leaves, leaves,
			leaves, leaves, wood, 	leaves, leaves,
			leaves, leaves, leaves, leaves, leaves,
			0, 		leaves, leaves, leaves, 0,
		},
		{
			0, 0, 		0, 		0,		0,
			0, leaves,	leaves, leaves, 0,
			0, leaves,	leaves, leaves, 0,
			0, leaves,	leaves, leaves, 0,
			0, 0, 		0, 		0, 		0,
		},
		{
			0, 0, 		0, 		0, 		0,
			0, 0, 		leaves, 0, 		0,
			0, leaves,	leaves, leaves, 0,
			0, 0, 		leaves, 0, 		0,
			0, 0, 		0, 		0, 		0,
		}
	}

	for idx = 1, #tree_def do
		for x_off = -2, 2 do
			for z_off = -2, 2 do
				local value = tree_def[idx][x_off + 3 + (z_off + 2) * 5]
				if value ~= 0 then
						map[calculateMapOffset(x + x_off, y + idx - 1, z + z_off)] = value
				end
			end
		end
	end
end ]]

--local tblpack = function(...)
--    return {n = select("#", ...), ...}
--end

local ffi = require("ffi")

-- Map generation
local generateMap = function(supplier, world_x, world_y, world_z)
	local map_data = supplier(world_x, world_z)
	local cast_map_data = ffi.cast("uint8_t*", map_data)
	
	for map_pos_x = 1, config.chunk_size_x do

		for map_pos_z = 1, config.chunk_size_z do

			for map_pos_y = 1, config.chunk_size_y do
				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

				map[offset] = cast_map_data[(map_pos_z - 1) + ((map_pos_y - 1) * config.chunk_size_x * config.chunk_size_z) + ((map_pos_x - 1) * config.chunk_size_z)]

--				map[offset] = 1

--				local noise_adjusted = math.floor((noise_raw + 1) * config.noise_intensity)
--				local randomY = noise_adjusted + config.floor_level
--
--				if map[offset] == nil then
--					if map_pos_y < randomY then
--						if (randomY - map_pos_y - 1) < 1 then
--							map[offset] = gamedefs.block_types.GRASS
--						elseif (randomY - map_pos_y - 1) < 3 then
--							map[offset] = gamedefs.block_types.DIRT
--						else
--							map[offset] = gamedefs.block_types.STONE
--						end
--					else
--						map[offset] = 0
--						if (map[offset - calculateMapOffsetZ(config.chunk_size_z)] == 2
--							and math.random(config.tree_generation_chance) > (config.tree_generation_chance - 2)
--							and map_pos_x > 2 and map_pos_x < (config.chunk_size_x - 1)
--							and map_pos_z > 2 and map_pos_z < (config.chunk_size_z - 1)) then
--							generateTree(map_pos_x, map_pos_y, map_pos_z)
--						end
--					end
--				end
			end
		end
	end
end

-- Mesh generation
local faceYielder = function(value, face, x_off, z_off, y_off)
	if value ~= nil then
		local normals = {
			{1, 0, 0},
			{-1, 0, 0},
			{0, 1, 0},
			{0, -1, 0},
			{0, 0, 1},
			{0, 0, -1}
		}

		local face_textures = { 1, 1, 0, 0, 1, 1 }
		local use_texture = face_textures[face]

		for tri = 1, 2 do
			for vert = 1, 3 do
				local x, y, z, u, v = unpack(dynagen_defs.face_defs[face][tri][vert])
				x = x + x_off - 1
				z = z + z_off - 1
				y = y + y_off - 1

				local u_ratio = 0.9 / gamedefs.block_type_count
				local v_ratio = 0.9 / gamedefs.block_textures

				-- ((size of 1 texture in atlas) * uv coord) + (offset from first texture) 
				local modified_u = (u_ratio * u)
				local modified_v = (v_ratio * v)

				local offset_u = modified_u + ((value - 1) / gamedefs.block_type_count) + (u_ratio / gamedefs.texture_pixels.x / 3)
				local offset_v = modified_v + (use_texture / gamedefs.block_textures) + (v_ratio / gamedefs.texture_pixels.y / 3)

				coroutine.yield(offset_u, offset_v, normals[face], {0, 0, 0}, x, y, z)
			end
		end
	end
end

GENERATOR_FUNCTION = function(supplier, world_x, world_y, world_z)
	map = {}

	--print(supplier)
	--print(supplier())

	generateMap(supplier, world_x, world_y, world_z)
	
	for map_pos_y = 1, config.chunk_size_y do
		for map_pos_z = 1, config.chunk_size_z do
			for map_pos_x = 1, config.chunk_size_x do

				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

				local map_val = map[offset]

				-- if current block is solid, check for boundaries
				if map_val ~= 0 then

					local map_next_x = 0
					if map_pos_x < config.chunk_size_x then map_next_x = map[offset + 1] end

					local map_prev_x = 0
					if map_pos_x > 1 then map_prev_x = map[offset - 1] end

					local map_next_z = 0
					if map_pos_z < config.chunk_size_z then map_next_z = map[offset + config.chunk_size_x] end

					local map_prev_z = 0
					if map_pos_z > 1 then map_prev_z = map[offset - config.chunk_size_x] end

					local map_next_y = 0
					if map_pos_y < config.chunk_size_y then map_next_y = map[offset + calculateMapOffsetZ(config.chunk_size_z)] end

					local map_prev_y = 0
					if map_pos_y > 1 then map_prev_y = map[offset - calculateMapOffsetZ(config.chunk_size_z)] end

					-- if current and next Y (vertical) position has air (block boundary)
					if map_next_y == 0 then
						faceYielder(map_val, dynagen_defs.faces.YPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if previous Y (vertical) position has air (block boundary)
					if map_prev_y == 0 then
						faceYielder(map_val, dynagen_defs.faces.YNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if next Z position has air (block boundary)
					if map_next_z == 0 then
						faceYielder(map_val, dynagen_defs.faces.ZPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if previous Z position has air (block boundary)
					if map_prev_z == 0 then
						faceYielder(map_val, dynagen_defs.faces.ZNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if next X position has air (block boundary)
					if map_next_x == 0 then
						faceYielder(map_val, dynagen_defs.faces.XPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if last X position has air (block boundary)
					if map_prev_x == 0 then
						faceYielder(map_val, dynagen_defs.faces.XNEG, map_pos_x, map_pos_z, map_pos_y)
					end
				end
			end
		end
	end
--	--print_trace()
--	--debug.sethook()
end