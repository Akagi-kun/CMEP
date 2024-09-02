local dynagen_defs = require("dynagen_defs")
local config = require("config")
local game_defs = require("game_defs")
local util = require("util")

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

--local tblpack = function(...)
--    return {n = select("#", ...), ...}
--end

-- Mesh generation
local faceYielder = function(value, face, x_off, z_off, y_off)
	if value ~= nil and value ~= 0 then
		local normals = {
			{1, 0, 0},
			{-1, 0, 0},
			{0, 1, 0},
			{0, -1, 0},
			{0, 0, 1},
			{0, 0, -1},

			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
		}

		local face_textures = { 1, 1, 0, 0, 1, 1, 0, 0, 0, 0 }
		local use_texture = face_textures[face]

		if value > game_defs.block_type_count then value = game_defs.block_type_count end

		for tri = 1, 2 do
			for vert = 1, 3 do
				local x, y, z, u, v = unpack(dynagen_defs.face_defs[face][tri][vert])
				x = x + x_off - 1
				z = z + z_off - 1
				y = y + y_off - 1

				local u_ratio = 0.8 / game_defs.block_type_count -- 0.9
				local v_ratio = 0.8 / game_defs.block_textures -- 0.9

				-- ((size of 1 texture in atlas) * uv coord) + (offset from first texture) 
				local modified_u = (u_ratio * u)
				local modified_v = (v_ratio * v)

				local block_type_offset = ((value - 1) / game_defs.block_type_count)
				local block_texture_offset = (use_texture / game_defs.block_textures)
				local pixel_offset = 1.25

				-- scaled UV + offset of texture + (pixel offset)
				local offset_u = modified_u + block_type_offset + (u_ratio / game_defs.texture_pixels.x * pixel_offset)
				local offset_v = modified_v + block_texture_offset + (v_ratio / game_defs.texture_pixels.y * pixel_offset)

				coroutine.yield(offset_u, offset_v, normals[face], {0, 0, 0}, x, y, z)
			end
		end
	end
end

local tblcontains = function(table, value)
	for k, v in ipairs(table) do
		if v == value then
			return true
		end
	end
	return false
end

local ffi = require("ffi")
local bit = require("bit")

generate_fn = function(supplier, world_x, world_y, world_z)

	local map_data = supplier(world_x, world_z)
	
	local map = ffi.cast("uint16_t*", map_data)
	
	for map_pos_y = 1, config.chunk_size_y do
		for map_pos_z = 1, config.chunk_size_z do
			for map_pos_x = 1, config.chunk_size_x do

				local offset = util.calculateMapOffset(map_pos_x, map_pos_y, map_pos_z)

				local map_val = map[offset]

				-- is_bordered = true if this chunk border ~= 0 and next/previous chunk border ~= 0
				local is_bordered_Z = false
				local is_bordered_X = false
				if bit.band(map_val, 128) > 0 then
					is_bordered_Z = true
					map_val = map_val - 128
				end
				if bit.band(map_val, 256) > 0 then
					is_bordered_X = true
					map_val = map_val - 256
				end

				if tblcontains(game_defs.blocks_crossfaced, map_val) then
					faceYielder(map_val, dynagen_defs.faces.CROSS_XNEG_ZNEG, map_pos_x, map_pos_z, map_pos_y)
					faceYielder(map_val, dynagen_defs.faces.CROSS_XPOS_ZNEG, map_pos_x, map_pos_z, map_pos_y)
					faceYielder(map_val, dynagen_defs.faces.CROSS_XPOS_ZPOS, map_pos_x, map_pos_z, map_pos_y)
					faceYielder(map_val, dynagen_defs.faces.CROSS_XNEG_ZPOS, map_pos_x, map_pos_z, map_pos_y)
					map_val = 0
				end

				-- if current block is solid, check for boundaries
				if not tblcontains(game_defs.blocks_transparent, map_val) then

					local map_next_x = 0
					if map_pos_x < config.chunk_size_x then map_next_x = map[offset + 1] end
					if is_bordered_X and map_pos_x == 16 then map_next_x = map_val end

					local map_prev_x = 0
					if map_pos_x > 1 then map_prev_x = map[offset - 1] end
					if is_bordered_X and map_pos_x == 1 then map_prev_x = map_val end

					local map_next_z = 0
					if map_pos_z < config.chunk_size_z then map_next_z = map[offset + config.chunk_size_x] end
					if is_bordered_Z and map_pos_z == 16 then map_next_z = map_val end

					local map_prev_z = 0
					if map_pos_z > 1 then map_prev_z = map[offset - config.chunk_size_x] end
					if is_bordered_Z and map_pos_z == 1 then map_prev_z = map_val end

					local map_next_y = 0
					if map_pos_y < config.chunk_size_y then map_next_y = map[offset + util.calculateMapOffsetY(1)] end

					local map_prev_y = 0
					if map_pos_y > 1 then map_prev_y = map[offset - util.calculateMapOffsetY(1)] end

					-- if current and next Y (vertical) position has air (block boundary)
					if tblcontains(game_defs.blocks_transparent, map_next_y) or
						tblcontains(game_defs.blocks_crossfaced, map_next_y) then
						faceYielder(map_val, dynagen_defs.faces.YPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if previous Y (vertical) position has air (block boundary)
					-- check only pos > 1 so that we avoid vertices on the bottom of the map
					if map_pos_y > 1 and
						(tblcontains(game_defs.blocks_transparent, map_prev_y) or
						tblcontains(game_defs.blocks_crossfaced, map_prev_y)) then
						faceYielder(map_val, dynagen_defs.faces.YNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if next Z position has air (block boundary)
					if tblcontains(game_defs.blocks_transparent, map_next_z) or
						tblcontains(game_defs.blocks_crossfaced, map_next_z) then
						faceYielder(map_val, dynagen_defs.faces.ZPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if previous Z position has air (block boundary)
					if tblcontains(game_defs.blocks_transparent, map_prev_z) or
						tblcontains(game_defs.blocks_crossfaced, map_prev_z) then
						faceYielder(map_val, dynagen_defs.faces.ZNEG, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if next X position has air (block boundary)
					if tblcontains(game_defs.blocks_transparent, map_next_x) or
						tblcontains(game_defs.blocks_crossfaced, map_next_x) then
						faceYielder(map_val, dynagen_defs.faces.XPOS, map_pos_x, map_pos_z, map_pos_y)
					end

					-- if last X position has air (block boundary)
					if tblcontains(game_defs.blocks_transparent, map_prev_x) or
						tblcontains(game_defs.blocks_crossfaced, map_prev_x) then
						faceYielder(map_val, dynagen_defs.faces.XNEG, map_pos_x, map_pos_z, map_pos_y)
					end
				end
			end
		end
	end
--	--print_trace()
--	--debug.sethook()
end