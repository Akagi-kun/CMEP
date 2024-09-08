local config = require("config")
local ffi = require("ffi")
local util = require("util")
local game_defs = require("game_defs")
require("perlin")

local cdefs = require("cdef")

-- Debugging data
local deltaTime_accum = 0.0
local deltaTime_count = 0
local deltaTime_max = 0.0
local deltaTime_min = 2000.0

local chunks_x = config.render_distance
local chunks_z = config.render_distance
local chunks = {}

-- Forward declaration
local generate_chunk

ffi.cdef[[
void *malloc( size_t size );
]]

local generateTree = function(map_data, x, y, z)
	for y_layer = 1, #(game_defs.tree_def) do
		for x_off = -2, 2 do
			for z_off = -2, 2 do
				local value = game_defs.tree_def[y_layer][x_off + 3 + (z_off + 2) * 5]

				if not util.validateX(x + x_off) then break end
				if not util.validateY(y + y_layer) then break end
				if not util.validateZ(z + z_off) then break end

				if value ~= 0 then
					local offset = util.calculateMapOffset(x + x_off, y + y_layer, z + z_off)
					map_data[offset] = value
				end
			end
		end
	end
end

-- Chunk boundary marking step
--
-- Optimize mesh generation by marking blocks that neighbor blocks in other chunks
-- so we can include the mesh only for "visible" edges of the chunks
--
-- without this it'd be necessary to build the mesh for all boundary blocks
-- making huge meshes that are mostly invisible
--
local generate_boundary_info = function(chunk_x, chunk_z)
	assert(chunks[chunk_x] ~= nil and chunks[chunk_x][chunk_z] ~= nil and chunks[chunk_x][chunk_z].data ~= nil)
	local this_chunk = chunks[chunk_x][chunk_z].data

	local bounds_check = chunk_z >= -chunks_z and chunk_z <= chunks_z and chunk_x >= -chunks_x and chunk_x <= chunks_x

	::before_check1::
	if chunks[chunk_x][chunk_z - 1] ~= nil and chunks[chunk_x][chunk_z - 1].data ~= nil then
		local prev_chunk = chunks[chunk_x][chunk_z - 1].data

		for x = 1, config.chunk_size_x do
			for y = 1, config.chunk_size_y do
				local offset1 = util.calculateMapOffset(x, y, 1)
				local offset2 = util.calculateMapOffset(x, y, config.chunk_size_z)
				if prev_chunk[offset2] ~= 0 and this_chunk[offset1] < 128 and this_chunk[offset1] ~= 0 then
					this_chunk[offset1] = this_chunk[offset1] + 128
				end
			end
		end
	else
		if bounds_check then
			generate_chunk(chunk_x, chunk_z - 1)
			goto before_check1
		end
	end

	::before_check2::
	if chunks[chunk_x][chunk_z + 1] ~= nil and chunks[chunk_x][chunk_z + 1].data ~= nil then
		local next_chunk = chunks[chunk_x][chunk_z + 1].data

		for x = 1, config.chunk_size_x do
			for y = 1, config.chunk_size_y do
				local offset1 = util.calculateMapOffset(x, y, config.chunk_size_z)
				local offset2 = util.calculateMapOffset(x, y, 1)
				if next_chunk[offset2] ~= 0 and this_chunk[offset1] < 128 and this_chunk[offset1] ~= 0 then
					this_chunk[offset1] = this_chunk[offset1] + 128
				end
			end
		end
	else
		if bounds_check then
			generate_chunk(chunk_x, chunk_z + 1)
			goto before_check2
		end
	end

	::before_check3::
	if chunks[chunk_x - 1] ~= nil and chunks[chunk_x - 1][chunk_z] ~= nil and chunks[chunk_x - 1][chunk_z].data ~= nil then
		local prev_chunk = chunks[chunk_x - 1][chunk_z].data

		for z = 1, config.chunk_size_z do
			for y = 1, config.chunk_size_y do
				local offset1 = util.calculateMapOffset(1, y, z)
				local offset2 = util.calculateMapOffset(config.chunk_size_x, y, z)
				if prev_chunk[offset2] ~= 0 and this_chunk[offset1] < 256 and this_chunk[offset1] ~= 0 then
					this_chunk[offset1] = this_chunk[offset1] + 256
				end
			end
		end
	else
		if bounds_check then
			generate_chunk(chunk_x - 1, chunk_z)
			goto before_check3
		end
	end

	::before_check4::
	if chunks[chunk_x + 1] ~= nil and chunks[chunk_x + 1][chunk_z] ~= nil and chunks[chunk_x + 1][chunk_z].data ~= nil then
		local next_chunk = chunks[chunk_x + 1][chunk_z].data

		for z = 1, config.chunk_size_z do
			for y = 1, config.chunk_size_y do
				local offset1 = util.calculateMapOffset(config.chunk_size_x, y, z)
				local offset2 = util.calculateMapOffset(1, y, z)
				if next_chunk[offset2] ~= 0 and this_chunk[offset1] < 256 and this_chunk[offset1] ~= 0 then
					this_chunk[offset1] = this_chunk[offset1] + 256
				end
			end
		end
	else
		if bounds_check then
			generate_chunk(chunk_x + 1, chunk_z)
			goto before_check4
		end
	end
end

generate_chunk = function(chunk_x, chunk_z)
	print("Attempting generate for chunk", chunk_x, chunk_z)
	local xpos = chunk_x * config.chunk_size_x
	local zpos = chunk_z * config.chunk_size_z

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
				local offset = util.calculateMapOffset(x, y, z)
				
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

	-- Tree generator step
	for tree_k, tree_v in ipairs(tree_placement_data) do
		generateTree(cast_map_data, tree_v[1], tree_v[2], tree_v[3])
	end

	-- Flower generator step
--	for flower_k, flower_v in ipairs(flower_placement_data) do
--		print(flower_k)
--		local offset = util.calculateMapOffset(flower_v[1], flower_v[2] + 1, flower_v[3])
--		cast_map_data[offset] = game_defs.block_types.FLOWER
--	end

	if chunks[chunk_x] == nil then chunks[chunk_x] = {} end
	if chunks[chunk_x][chunk_z] == nil then chunks[chunk_x][chunk_z] = {} end
	chunks[chunk_x][chunk_z].data = cast_map_data

	generate_boundary_info(chunk_x, chunk_z)
end

terrain_generator = function(...)
	local xpos = select(1, ...)
	local zpos = select(2, ...)

	local chunk_x = xpos / config.chunk_size_x
	local chunk_z = zpos / config.chunk_size_z

	if chunks[chunk_x] == nil or chunks[chunk_x][chunk_z] == nil or chunks[chunk_x][chunk_z].data == nil then
		print("Generator supplier could not find chunk data! Regenerating")
		generate_chunk(chunk_x, chunk_z)
	end

	return assert(chunks[chunk_x][chunk_z].data, "No chunk buffer exists for this chunk!")
end

--check_chunks_loaded = function(asset_manager, scene)
--	for chunk_x = -chunks_x, chunks_x, 1 do
--		if chunks[chunk_x] == nil then chunks[chunk_x] = {} end
--		for chunk_z = -chunks_z, chunks_z, 1 do
--			if chunks[chunk_x][chunk_z] == nil then
--
--				local chunk_obj = engine.createSceneObject(asset_manager, "renderer_3d/generator", "terrain", {
--					{"texture", "atlas"}, {"generator_script", "testgen"}, {"generator_supplier", "script0/terrain_generator"}
--				})
--				chunk_obj:SetPosition((chunk_x) * config.chunk_size_x, 0.0, (chunk_z) * config.chunk_size_z)
--				chunk_obj:SetSize(1, 1, 1)
--				chunk_obj:SetRotation(0, 0, 0)
--				scene:AddObject(string.format("chunk_%i_%i", chunk_x, chunk_z), chunk_obj)
--				engine.RendererForceBuild(chunk_obj.renderer)
--				chunks[chunk_x][chunk_z] = {chunk_obj}
--
--				coroutine.yield()
--			end
--		end
--	end
--end

--load_chunks_coro = coroutine.create(check_chunks_loaded)

-- ON_UPDATE event
-- 
-- called every frame
--
-- event.deltaTime (float) contains the delta time in microseconds
-- and is the period between the last onUpdate and the current one
--
onUpdate = function(event)
	deltaTime_accum = deltaTime_accum + event.deltaTime

	deltaTime_max = math.max(deltaTime_max, event.deltaTime)
	deltaTime_min = math.min(deltaTime_min, event.deltaTime)

	deltaTime_count = deltaTime_count + 1

	local asset_manager = event.engine:getAssetManager()
	local scene_manager = event.engine:getSceneManager()
	local scene = scene_manager:getSceneCurrent()

	--[[ if coroutine.status(load_chunks_coro) ~= "dead" then
		coroutine.resume(load_chunks_coro, asset_manager, scene)
	end ]]

	-- Updates frametime counter, recommend to leave this here for debugging purposes
	if deltaTime_accum >= 1.0 then
		local deltaTime_avg = deltaTime_accum / deltaTime_count
		local object = scene:findObject("_debug_info")

		meshBuilderSupplyData(object.meshbuilder, "text", string.format("avg: %fms\nmin: %fms\nmax: %fms", deltaTime_avg * 1000, deltaTime_min * 1000, deltaTime_max * 1000))

		deltaTime_min = 2000.0
		deltaTime_max = 0.0
		deltaTime_accum = 0
		deltaTime_count = 0
	end

	local camx, camy, camz = scene_manager:getCameraTransform()
	local camh, camv = scene_manager:getCameraRotation()

	local dbg2 = scene:findObject("_debug_info2")
	meshBuilderSupplyData(dbg2.meshbuilder, "text", string.format("H: %f V: %f\nX: %f Y: %f Z: %f", camh, camv, camx, camy, camz))

	return 0
end

-- ON_INIT event
--
-- this event is fired after the scene is loaded and before the first rendered frame
-- perform initialization here
--
onInit = function(event)
	-- Get managers
	local asset_manager = event.engine:getAssetManager()
	local scene_manager = event.engine:getSceneManager()
	local scene = scene_manager:getSceneCurrent()

	local font = asset_manager:getFont("myfont")

	-- Set-up camera
	scene_manager:setCameraTransform(-1.0, 55.8, 2.5)
	scene_manager:setCameraRotation(114.0, 224.8)

	-- Create frametime counter and add it to scene
	local object0 = createSceneObject(event.engine, "renderer_2d/text", "text",
		{ {"font", font} }, { {"text", "avg: \nmin: \nmax: "} }
	)
	object0:setPosition(0.0, 0.0, -0.01)
	object0:setSize(24, 24, 1.0)
	scene:addObject("_debug_info", object0)
	
	local object1 = createSceneObject(event.engine, "renderer_2d/text", "text",
		{ {"font", font} }, { {"text", "H: V: \nX: Y: Z: "} }
	)
	object1:setPosition(0.6, 0.0, -0.01)
	object1:setSize(24, 24, 1.0)
	scene:addObject("_debug_info2", object1)

	print("Generating chunks...")
	generate_chunk(0, 0)
	collectgarbage()

	local testgen_script = asset_manager:getScript("testgen")
	local supplier_script = asset_manager:getScript("script0")

	local atlas_texture = asset_manager:getTexture("atlas")

	for chunk_x = -chunks_x, chunks_x, 1 do
		for chunk_z = -chunks_z, chunks_z, 1 do
			local chunk_obj = createSceneObject(event.engine, "renderer_3d/generator", "terrain",
				{ {"texture", atlas_texture} }, { {"generator", cdefs.CreateGeneratorData(testgen_script, "generate_fn", supplier_script, "terrain_generator")} }
			)
			chunk_obj:setPosition(chunk_x * config.chunk_size_x, 0.0, chunk_z * config.chunk_size_z)
			chunk_obj:setSize(1, 1, 1)
			chunk_obj:setRotation(0, 0, 0)
			scene:addObject(string.format("chunk_%i_%i", chunk_x, chunk_z), chunk_obj)
			chunks[chunk_x][chunk_z].object = {chunk_obj}
		end
	end

	return 0
end
