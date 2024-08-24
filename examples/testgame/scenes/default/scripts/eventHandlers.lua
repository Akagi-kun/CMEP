local config = require("config")
local ffi = require("ffi")
local game_defs = require("game_defs")
require("perlin")

-- Debugging data
local deltaTime_accum = 0.0
local deltaTime_count = 0
local deltaTime_max = 0.0
local deltaTime_min = 2000.0

local chunks_x = config.render_distance
local chunks_z = config.render_distance
local chunks = {}

ffi.cdef[[
void *malloc( size_t size );
]]

local calculateMapOffsetY = function(y)
	return ((y) * config.chunk_size_x * config.chunk_size_z)
end

local calculateMapOffset = function(x, y, z)
	return (x - 1) + calculateMapOffsetY(y - 1) + ((z - 1) * config.chunk_size_x)
end

local generateTree = function(map_data, x, y, z)
	for y_layer = 1, #(game_defs.tree_def) do
		for x_off = -2, 2 do
			for z_off = -2, 2 do
				local value = game_defs.tree_def[y_layer][x_off + 3 + (z_off + 2) * 5]

				if value ~= 0 then
					local offset = calculateMapOffset(x + x_off, y + y_layer, z + z_off)
					map_data[offset] = value
				end
			end
		end
	end
end

local generate_chunk = function(xpos, zpos)
	local size = config.chunk_size_x * config.chunk_size_y * config.chunk_size_z
	local map_data = ffi.C.malloc(ffi.sizeof("uint16_t") * size)
	assert(map_data, "Could not allocate chunk buffer")
	local cast_map_data = ffi.cast("uint16_t*", map_data)

	local tree_placement_data = {}
	local flower_placement_data = {}

	for z = 1, config.chunk_size_z do
		for x = 1, config.chunk_size_x do
			local noise_raw = perlin:noise((x + xpos) / config.chunk_size_x / 2, config.noise_layer, (z + zpos) / config.chunk_size_z / 2)
			local noise_raw2 = perlin:noise((x + xpos) / config.chunk_size_x / 6, config.noise_layer + 0.4, (z + zpos) / config.chunk_size_z / 6)
			local noise_raw3 = perlin:noise((x + xpos) / config.chunk_size_x / 10, config.noise_layer + 0.6, (z + zpos) / config.chunk_size_z / 10)

			--print(noise_raw, noise_raw2)

			local noise_adjusted1 = (noise_raw + 1) * (config.noise_intensity / 2)
			local noise_adjusted2 = (noise_raw2 + 1) * (config.noise_intensity / 1.2)
			local noise_adjusted3 = (noise_raw3 + 1) * (config.noise_intensity)
			local noise_adjusted = math.floor(noise_adjusted1 / 3 + noise_adjusted2 / 3 + noise_adjusted3 / 3)
			local random_y = noise_adjusted + config.floor_level

			--print(string.format("[%i, %i, %f, %f]", x + xpos, z + zpos, noise_adjusted1, noise_adjusted2))

			for y = 1, config.chunk_size_y do
				local offset = calculateMapOffset(x, y, z)
				
				-- Primary terrain generator step
				if y > random_y then
					cast_map_data[offset] = game_defs.block_types.AIR
				else
					if y == random_y then
						if random_y > config.water_table_level then
							cast_map_data[offset] = game_defs.block_types.GRASS
						
							-- Tree placement selector
							if (math.random(config.tree_generation_chance) > (config.tree_generation_chance - 2)
								and x > 2 and x < (config.chunk_size_x - 1)
								and z > 2 and z < (config.chunk_size_z - 1)) then
								table.insert(tree_placement_data, {x, y, z})
							-- Flower placement selector
							elseif (math.random(config.flower_generation_chance) > (config.flower_generation_chance - 2)
								and x > 2 and x < (config.chunk_size_x - 1)
								and z > 2 and z < (config.chunk_size_z - 1)) then
								table.insert(flower_placement_data, {x, y, z})
							end
						else
							cast_map_data[offset] = game_defs.block_types.SAND -- water areas?
						end
					else
						if y > (random_y - 3) then
							cast_map_data[offset] = game_defs.block_types.DIRT
						else
							cast_map_data[offset] = game_defs.block_types.STONE
						end
					end
				end
			end
		end
	end

	-- Places trees at selected positions
	for tree_k, tree_v in ipairs(tree_placement_data) do
		generateTree(cast_map_data, tree_v[1], tree_v[2], tree_v[3])
	end

	-- Places flowers at selected positions
--	for flower_k, flower_v in ipairs(flower_placement_data) do
--		print(flower_k)
--		local offset = calculateMapOffset(flower_v[1], flower_v[2] + 1, flower_v[3])
--		cast_map_data[offset] = game_defs.block_types.FLOWER
--	end

	local chunk_x = xpos / config.chunk_size_x
	local chunk_z = zpos / config.chunk_size_z

--	local this_chunk = cast_map_data

	if chunks[chunk_x] ~= nil and chunks[chunk_x][chunk_z - 1] ~= nil and chunks[chunk_x][chunk_z - 1].data ~= nil then
		local prev_chunk = chunks[chunk_x][chunk_z - 1].data

		for x = 1, config.chunk_size_x do
			for y = 1, config.chunk_size_y do
				local offset1 = calculateMapOffset(x, y, 1)
				local offset2 = calculateMapOffset(x, y, config.chunk_size_z)
				if prev_chunk[offset2] ~= 0 and cast_map_data[offset1] < 128 and cast_map_data[offset1] ~= 0 then
					cast_map_data[offset1] = cast_map_data[offset1] + 128
				end
			end
		end
	end
	if chunks[chunk_x] ~= nil and chunks[chunk_x][chunk_z + 1] ~= nil and chunks[chunk_x][chunk_z + 1].data ~= nil then
		local next_chunk = chunks[chunk_x][chunk_z + 1].data

		for x = 1, config.chunk_size_x do
			for y = 1, config.chunk_size_y do
				local offset1 = calculateMapOffset(x, y, config.chunk_size_z)
				local offset2 = calculateMapOffset(x, y, 1)
				if next_chunk[offset2] ~= 0 and cast_map_data[offset1] < 128 and cast_map_data[offset1] ~= 0 then
					cast_map_data[offset1] = cast_map_data[offset1] + 128
				end
			end
		end
	end

	if chunks[chunk_x - 1] ~= nil and chunks[chunk_x - 1][chunk_z] ~= nil and chunks[chunk_x - 1][chunk_z].data ~= nil then
		local prev_chunk = chunks[chunk_x - 1][chunk_z].data

		for z = 1, config.chunk_size_z do
			for y = 1, config.chunk_size_y do
				local offset1 = calculateMapOffset(1, y, z)
				local offset2 = calculateMapOffset(config.chunk_size_x, y, z)
				if prev_chunk[offset2] ~= 0 and cast_map_data[offset1] < 256 and cast_map_data[offset1] ~= 0 then
					cast_map_data[offset1] = cast_map_data[offset1] + 256
				end
			end
		end
	end
	if chunks[chunk_x + 1] ~= nil and chunks[chunk_x + 1][chunk_z] ~= nil and chunks[chunk_x + 1][chunk_z].data ~= nil then
		local next_chunk = chunks[chunk_x + 1][chunk_z].data

		for z = 1, config.chunk_size_z do
			for y = 1, config.chunk_size_y do
				local offset1 = calculateMapOffset(config.chunk_size_x, y, z)
				local offset2 = calculateMapOffset(1, y, z)
				if next_chunk[offset2] ~= 0 and cast_map_data[offset1] < 256 and cast_map_data[offset1] ~= 0 then
					cast_map_data[offset1] = cast_map_data[offset1] + 256
				end
			end
		end
	end
	
	if chunks[chunk_x] == nil then chunks[chunk_x] = {} end
	if chunks[chunk_x][chunk_z] == nil then chunks[chunk_x][chunk_z] = {} end
	chunks[chunk_x][chunk_z].data = cast_map_data
end

terrain_generator = function(...)
	local xpos = select(1, ...)
	local zpos = select(2, ...)

	local chunk_x = xpos / config.chunk_size_x
	local chunk_z = zpos / config.chunk_size_z

	generate_chunk(xpos, zpos)

	assert(chunks[chunk_x][chunk_z].data, "No chunk buffer exists for this chunk!")

	return chunks[chunk_x][chunk_z].data
end

check_chunks_loaded = function(asset_manager, scene)
	for chunk_x = -chunks_x, chunks_x, 1 do
		if chunks[chunk_x] == nil then chunks[chunk_x] = {} end
		for chunk_z = -chunks_z, chunks_z, 1 do
			if chunks[chunk_x][chunk_z] == nil then
--				print(chunk_x, chunk_z);

				local chunk_obj = engine.CreateSceneObject(asset_manager, "renderer_3d/generator", "terrain", {
					{"texture", "atlas"}, {"generator_script", "testgen"}, {"generator_supplier", "script0/terrain_generator"}
				})
				chunk_obj:SetPosition((chunk_x) * config.chunk_size_x, 0.0, (chunk_z) * config.chunk_size_z)
				chunk_obj:SetSize(1, 1, 1)
				chunk_obj:SetRotation(0, 0, 0)
				scene:AddObject(string.format("chunk_%i_%i", chunk_x, chunk_z), chunk_obj)
				engine.RendererForceBuild(chunk_obj.renderer)
				chunks[chunk_x][chunk_z] = {chunk_obj}

				coroutine.yield()
			end
		end
	end
end

load_chunks_coro = coroutine.create(check_chunks_loaded)

-- ON_UPDATE event
-- 
-- called every frame
--
-- event.deltaTime (float) contains the delta time in microseconds
-- and is the period between the last onUpdate and the current one
--
onUpdate = function(event)
	--return 0
	deltaTime_accum = deltaTime_accum + event.deltaTime

	deltaTime_max = math.max(deltaTime_max, event.deltaTime)
	deltaTime_min = math.min(deltaTime_min, event.deltaTime)

	deltaTime_count = deltaTime_count + 1

	local asset_manager = event.engine:GetAssetManager()
	local scene_manager = event.engine:GetSceneManager()
	local scene = scene_manager:GetSceneCurrent()

	--[[ if coroutine.status(load_chunks_coro) ~= "dead" then
		coroutine.resume(load_chunks_coro, asset_manager, scene)
	end ]]

	--onMovementTick(event.deltaTime, scene_manager)

	-- Updates frametime counter, recommend to leave this here for debugging purposes
	if deltaTime_accum >= 1.0 then
		local deltaTime_avg = deltaTime_accum / deltaTime_count
		local object = scene:FindObject("_debug_info")

		engine.RendererSupplyText(object.renderer, string.format("avg: %fms\nmin: %fms\nmax: %fms", deltaTime_avg * 1000, deltaTime_min * 1000, deltaTime_max * 1000))

		deltaTime_min = 2000.0
		deltaTime_max = 0.0
		deltaTime_accum = 0
		deltaTime_count = 0
	end

	local camx, camy, camz = scene_manager:GetCameraTransform()
	local camh, camv = scene_manager:GetCameraHVRotation()

	local dbg2 = scene:FindObject("_debug_info2");
	engine.RendererSupplyText(dbg2.renderer, string.format("H: %f V: %f\nX: %f Y: %f Z: %f", camh, camv, camx, camy, camz))

	return 0
end

-- ON_INIT event
--
-- this event is fired after the scene is loaded and before the first rendered frame
-- perform initialization here
--
onInit = function(event)
	-- Get managers
	local asset_manager = event.engine:GetAssetManager()
	local scene_manager = event.engine:GetSceneManager()
	local scene = scene_manager:GetSceneCurrent();

	-- Set-up camera
	scene_manager:SetCameraTransform(-1.0, 55.8, 2.5)
	scene_manager:SetCameraHVRotation(114.0, 224.8)
	
	-- Create frametime counter and add it to scene
	local object0 = engine.CreateSceneObject(asset_manager, "renderer_2d/text", "text", {
		{"font", "myfont"}, {"text", "avg: \nmin: \nmax: "}
	  })
	object0:SetPosition(0.0, 0.0, -0.01)
	object0:SetSize(24, 24, 1.0)
	scene:AddObject("_debug_info", object0)
	
	local object1 = engine.CreateSceneObject(asset_manager, "renderer_2d/text", "text", {
		{"font", "myfont"}, {"text", "H: V: \nX: Y: Z: "}
	  })
	object1:SetPosition(0.6, 0.0, -0.01)
	object1:SetSize(24, 24, 1.0)
	scene:AddObject("_debug_info2", object1)

	--local object3 = engine.CreateSceneObject(asset_manager, "renderer_3d/sprite", "sprite", {
	--	{"texture", "sprite0"}
	--})
	--object3:SetPosition(-2.0, 1.0, 0.0)
	--object3:SetSize(1, 1, 1)
	--object3:SetRotation(0, -100, 180)
	--scene:AddObject("test3dsprite", object3)

	print("Generating chunks...")
	for chunk_x = -chunks_x, chunks_x, 1 do -- chunks_x, chunks_x, 1
--		chunks[chunk_x] = {}
		for chunk_z = -chunks_z, chunks_z, 1 do
--			chunks[chunk_x][chunk_z] = {}

			--print("Generating chunk "..chunk_x.." "..chunk_z)
			generate_chunk(chunk_x * config.chunk_size_x, chunk_z * config.chunk_size_z)
		end
	end

	for chunk_x = -chunks_x, chunks_x, 1 do -- chunks_x, chunks_x, 1
		--chunks[chunk_x] = {}
		for chunk_z = -chunks_z, chunks_z, 1 do
			local chunk_obj = engine.CreateSceneObject(asset_manager, "renderer_3d/generator", "terrain", {
				{"texture", "atlas"}, {"generator_script", "testgen"}, {"generator_supplier", "script0/terrain_generator"}
			})
			chunk_obj:SetPosition(chunk_x * config.chunk_size_x, 0.0, chunk_z * config.chunk_size_z)
			chunk_obj:SetSize(1, 1, 1)
			chunk_obj:SetRotation(0, 0, 0)
			scene:AddObject(string.format("chunk_%i_%i", chunk_x, chunk_z), chunk_obj)
			chunks[chunk_x][chunk_z].object = {chunk_obj}
			--print(string.format("Building chunk [%i,%i]",chunk_x, chunk_z))
			--engine.RendererForceBuild(chunk_obj.renderer)
		end
	end

	return 0
end
