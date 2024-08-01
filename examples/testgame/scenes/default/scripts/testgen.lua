require("perlin")

local faces = {
	XPOS = 1,
	XNEG = 2,
	YPOS = 3,
	YNEG = 4,
	ZPOS = 5,
	ZNEG = 6
}

local face_defs = {
	{ -- facing positive X
		{
			{1.0, 1.0, 0.0, 1.0, 0.0},
			{1.0, 1.0, 1.0, 0.0, 0.0},
			{1.0, 0.0, 0.0, 1.0, 1.0}
		},
		{
			{1.0, 0.0, 0.0, 1.0, 1.0},
			{1.0, 1.0, 1.0, 0.0, 0.0},
			{1.0, 0.0, 1.0, 0.0, 1.0}
		}
	},
	{ -- facing negative X
		{
			{0.0, 0.0, 0.0, 0.0, 1.0},
			{0.0, 1.0, 1.0, 1.0, 0.0},
			{0.0, 1.0, 0.0, 0.0, 0.0}
		},
		{
			{0.0, 0.0, 1.0, 1.0, 1.0},
			{0.0, 1.0, 1.0, 1.0, 0.0},
			{0.0, 0.0, 0.0, 0.0, 1.0}
		}
	},

	{ -- facing positive Y
		{
			{0.0, 1.0, 1.0, 0.0, 1.0},
			{1.0, 1.0, 1.0, 1.0, 1.0},
			{0.0, 1.0, 0.0, 0.0, 0.0}
		},
		{
			{0.0, 1.0, 0.0, 0.0, 0.0},
			{1.0, 1.0, 1.0, 1.0, 1.0},
			{1.0, 1.0, 0.0, 1.0, 0.0}
		}
	},
	{ -- facing negative Y
		{
			{0.0, 0.0, 0.0, 0.0, 1.0},
			{1.0, 0.0, 1.0, 1.0, 0.0},
			{0.0, 0.0, 1.0, 0.0, 0.0}
		},
		{
			{1.0, 0.0, 0.0, 1.0, 1.0},
			{1.0, 0.0, 1.0, 1.0, 0.0},
			{0.0, 0.0, 0.0, 0.0, 1.0}
		}
	},

	{ -- facing positive Z
		{
			{0.0, 0.0, 1.0, 0.0, 1.0},
			{1.0, 1.0, 1.0, 1.0, 0.0},
			{0.0, 1.0, 1.0, 0.0, 0.0}
		},
		{	{0.0, 0.0, 1.0, 0.0, 1.0},
			{1.0, 0.0, 1.0, 1.0, 1.0},
			{1.0, 1.0, 1.0, 1.0, 0.0}
		}
	},
	{ -- facing negative Z
		{
			{0.0, 1.0, 0.0, 1.0, 0.0},
			{1.0, 1.0, 0.0, 0.0, 0.0},
			{0.0, 0.0, 0.0, 1.0, 1.0}
		},
		{
			{1.0, 1.0, 0.0, 0.0, 0.0},
			{1.0, 0.0, 0.0, 0.0, 1.0},
			{0.0, 0.0, 0.0, 1.0, 1.0}
		}
	}
}

local map = {}
map.size_x = 16
map.size_z = 16
map.size_y = 32

local calculateMapOffsetZ = function(z)
	return (map.size_x * z)
end
local map_z_offset = calculateMapOffsetZ(map.size_z)

local calculateMapOffset = function(x, y, z)
	return x + (calculateMapOffsetZ(z - 1)) + (map_z_offset) * (y - 1)
end

local noise_intensity = 5
local noise_layer = 0.25
local floor_divider = 4

-- Map generation
local generateMap = function()

	for map_pos_y = 1, map.size_y do
		for map_pos_z = 1, map.size_z do
			for map_pos_x = 1, map.size_x do
--				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)
--				map[offset] = 1

				local floor_level = math.floor(map.size_y / floor_divider)
				local noise_raw = perlin:noise(map_pos_x / map.size_x, noise_layer, map_pos_z / map.size_z)
				local noise_adjusted = math.floor((noise_raw + 1) * noise_intensity)
				local randomY = noise_adjusted + floor_level

				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

				if map_pos_y < randomY then
					if (randomY - map_pos_y - 1) < 1 then
						map[offset] = 2
					elseif (randomY - map_pos_y - 1) < 3 then
						map[offset] = 3
					else
						--if math.random(100) > 95 then
						--	map[offset] = 4
						--else
							map[offset] = 1
						--end
					end
				else
					map[offset] = 0
				end
			end
		end
	end
end

local block_type_count = 4
local block_textures = 2

-- Mesh generation
local faceYielder = function(value, face, x_off, z_off, y_off)
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
			local x, y, z, u, v = unpack(face_defs[face][tri][vert])
			x = x + x_off - 1
			z = z + z_off - 1
			y = y + y_off - 1

			local modified_u = (1 / block_type_count * u) + ((value - 1) / block_type_count)
			local modified_v = (1 / block_textures * v) + (use_texture / block_textures)

			coroutine.yield({modified_u, modified_v}, colors[face], {x, y, z})
		end
	end
end

GENERATOR_FUNCTION = function()
	generateMap()
	
	for map_pos_y = 1, map.size_y do
		for map_pos_z = 1, map.size_z do
			for map_pos_x = 1, map.size_x do

				local offset = calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

				local map_val = map[offset]

				local map_next_x = 0
				if map_pos_x < map.size_x then map_next_x = map[offset + 1] end

				local map_prev_x = 0
				if map_pos_x > 1 then map_prev_x = map[offset - 1] end

				local map_next_z = 0
				if map_pos_z < map.size_z then map_next_z = map[offset + map.size_x] end

				local map_prev_z = 0
				if map_pos_z > 1 then map_prev_z = map[offset - map.size_x] end

				local map_next_y = 0
				if map_pos_y < map.size_y then map_next_y = map[offset + map_z_offset] end

				local map_prev_y = 0
				if map_pos_y > 1 then map_prev_y = map[offset - map_z_offset] end

				-- if current block is solid, check for boundaries
				if map_val ~= 0 then 
					-- if current and next Y (vertical) position differ (block boundary)
					if map_val ~= map_next_y then
						faceYielder(map_val, faces.YPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and previous Y (vertical) position differ (block boundary)
					if map_val ~= map_prev_y then
						faceYielder(map_val, faces.YNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and next Z position differ (block boundary)
					if map_val ~= map_next_z then
						faceYielder(map_val, faces.ZPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and previous Z position differ (block boundary)
					if map_val ~= map_prev_z then
						faceYielder(map_val, faces.ZNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and next X position differ (block boundary)
					if map_val ~= map_next_x then
						faceYielder(map_val, faces.XPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if current and last X position differ (block boundary)
					if map_val ~= map_prev_x then
						faceYielder(map_val, faces.XNEG, map_pos_x, map_pos_z, map_pos_y)
					end
				end
			end
		end
	end
end