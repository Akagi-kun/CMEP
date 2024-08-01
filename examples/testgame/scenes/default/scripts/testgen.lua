require("perlin")
require("dynagen_defs")

local map = {}
local map_size_x = 16
local map_size_z = 16
local map_size_y = 32

local calculateMapOffsetZ = function(z)
	return (map_size_x * z)
end

local map_z_offset = calculateMapOffsetZ(map_size_z)

local calculateMapOffset = function(x, y, z)
	return x + (calculateMapOffsetZ(z - 1)) + (map_z_offset) * (y - 1)
end

local noise_intensity = 10
local noise_layer = 0.0
local floor_level = 8

local generateTree = function(x, y, z)
	local tree_def = {
		{
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 6, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
		},
		{
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 6, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
		},
		{
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 6, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
		},
		--{
		--	0, 5, 5, 5, 0,
		--	5, 5, 5, 5, 5,
		--	5, 5, 6, 5, 5,
		--	5, 5, 5, 5, 5,
		--	0, 5, 5, 5, 0,
		--},
		--{
		--	0, 5, 5, 5, 0,
		--	5, 5, 5, 5, 5,
		--	5, 5, 6, 5, 5,
		--	5, 5, 5, 5, 5,
		--	0, 5, 5, 5, 0,
		--},
		--{
		--	0, 0, 0, 0, 0,
		--	0, 5, 5, 5, 0,
		--	0, 5, 5, 5, 0,
		--	0, 5, 5, 5, 0,
		--	0, 0, 0, 0, 0,
		--},
		--{
		--	0, 0, 0, 0, 0,
		--	0, 0, 5, 0, 0,
		--	0, 5, 5, 5, 0,
		--	0, 0, 5, 0, 0,
		--	0, 0, 0, 0, 0,
		--}
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
end

-- Map generation
local generateMap = function(world_x, world_y, world_z)
	for map_pos_x = 1, map_size_x do
		for map_pos_z = 1, map_size_z do
			local noise_raw = perlin:noise((map_pos_x + world_x) / map_size_x, noise_layer, (map_pos_z + world_z) / map_size_z)

			for map_pos_y = 1, map_size_y do
				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

--				map[offset] = 1

				local noise_adjusted = math.floor((noise_raw + 1) * noise_intensity)
				local randomY = noise_adjusted + floor_level

				if map[offset] == nil then
					if map_pos_y < randomY then
						if (randomY - map_pos_y - 1) < 1 then
							map[offset] = 2
						elseif (randomY - map_pos_y - 1) < 3 then
							map[offset] = 3
						else
							map[offset] = 1
						end
					else
						map[offset] = 0
						if (map[offset - map_z_offset] == 2 and math.random(360) > 358) then
							generateTree(map_pos_x, map_pos_y, map_pos_z)
						end
					end
				end
			end
		end
	end
end

-- Mesh generation
local block_type_count = 6
local block_textures = 2

local faceYielder = function(value, face, x_off, z_off, y_off)
	if value ~= nil then
		local colors = {
			{1, 0, 0},
			{1, 0, 0},
			{0, 1, 0},
			{0, 1, 0},
			{0, 0, 1},
			{0, 0, 1}
		}

		local face_textures = { 1, 1, 0, 0, 1, 1 }
		local use_texture = face_textures[face]

		for tri = 1, 2 do
			for vert = 1, 3 do
				local x, y, z, u, v = unpack(dynagen_defs.face_defs[face][tri][vert])
				x = x + x_off - 1
				z = z + z_off - 1
				y = y + y_off - 1

				local modified_u = (1 / block_type_count * u) + ((value - 1) / block_type_count)
				local modified_v = (1 / block_textures * v) + (use_texture / block_textures)

				coroutine.yield({modified_u, modified_v}, colors[face], {x, y, z})
			end
		end
	end
end

GENERATOR_FUNCTION = function(world_x, world_y, world_z)
	map = {}

	generateMap(world_x, world_y, world_z)
	
	for map_pos_y = 1, map_size_y do
		for map_pos_z = 1, map_size_z do
			for map_pos_x = 1, map_size_x do

				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

				local map_val = map[offset]

				local map_next_x = 0
				if map_pos_x < map_size_x then map_next_x = map[offset + 1] end

				local map_prev_x = 0
				if map_pos_x > 1 then map_prev_x = map[offset - 1] end

				local map_next_z = 0
				if map_pos_z < map_size_z then map_next_z = map[offset + map_size_x] end

				local map_prev_z = 0
				if map_pos_z > 1 then map_prev_z = map[offset - map_size_x] end

				local map_next_y = 0
				if map_pos_y < map_size_y then map_next_y = map[offset + map_z_offset] end

				local map_prev_y = 0
				if map_pos_y > 1 then map_prev_y = map[offset - map_z_offset] end

				-- if current block is solid, check for boundaries
				if map_val ~= 0 then 
					-- if current and next Y (vertical) position differ (block boundary)
					if map_val ~= map_next_y then
						faceYielder(map_val, dynagen_defs.faces.YPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and previous Y (vertical) position differ (block boundary)
					if map_val ~= map_prev_y then
						faceYielder(map_val, dynagen_defs.faces.YNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and next Z position differ (block boundary)
					if map_val ~= map_next_z then
						faceYielder(map_val, dynagen_defs.faces.ZPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and previous Z position differ (block boundary)
					if map_val ~= map_prev_z then
						faceYielder(map_val, dynagen_defs.faces.ZNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and next X position differ (block boundary)
					if map_val ~= map_next_x then
						faceYielder(map_val, dynagen_defs.faces.XPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and last X position differ (block boundary)
					if map_val ~= map_prev_x then
						faceYielder(map_val, dynagen_defs.faces.XNEG, map_pos_x, map_pos_z, map_pos_y)
					end
				end
			end
		end
	end
end