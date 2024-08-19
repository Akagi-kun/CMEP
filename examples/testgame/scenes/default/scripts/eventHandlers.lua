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
	local leaves = game_defs.block_types.LEAVES
	local wood = game_defs.block_types.WOOD
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

	for y_layer = 1, #tree_def do
		for x_off = -2, 2 do
			for z_off = -2, 2 do
				local value = tree_def[y_layer][x_off + 3 + (z_off + 2) * 5]

				if value ~= 0 then
					local offset = calculateMapOffset(x + x_off, y + y_layer, z + z_off)
					map_data[offset] = value
				end
			end
		end
	end
end

terrain_generator = function(...)
	local xpos = select(1, ...)
	local zpos = select(2, ...)

	local map_data = ffi.C.malloc(ffi.sizeof("uint8_t") * config.chunk_size_x * config.chunk_size_y * config.chunk_size_z)
	local cast_map_data = ffi.cast("uint8_t*", map_data)

	local tree_placement_data = {}

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

	--print(xpos, zpos, noise_val)

	return --[[ cast_data, ]] cast_map_data
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
				chunks[chunk_x][chunk_z] = chunk_obj

				coroutine.yield()
			end
		end
	end
end

-- ON_MOUSEMOVED event
-- 
-- this event is called when the mouse moved
--
-- while specifying event handlers is optional
-- it is left here for illustration purposes
-- (events for which no event handler is specified are discarded)
-- 
onMouseMoved = function(event)
	local mouseSpeed = 5.0;

	local scene_manager = event.engine:GetSceneManager()

	local h, v = scene_manager:GetCameraHVRotation()

	--engine.logger.SimpleLog(string.format("X: %f Y: %f", event.mouse.x, event.mouse.y))

	h = h + (mouseSpeed * event.deltaTime) * event.mouse.x
	v = v + (mouseSpeed * event.deltaTime) * event.mouse.y

	scene_manager:SetCameraHVRotation(h, v)

	return 0
end

local vectorCross = function(v1x, v1y, v1z, v2x, v2y, v2z)
	-- x		= v1.y * v2.z - v2.y * v1.z
	local out_x = v1y  * v2z  - v2y  * v1z;
	
	-- y		= v2.x * v1.z - v1.x * v2.z
	local out_y = v2x  * v1z  - v1x  * v2z;

	-- z		= v1.x * v2.y - v2.x * v1.y
	local out_z = v1x  * v2y  - v2x  * v1y;

	return out_x, out_y, out_z
end

-- ON_KEYDOWN event
-- 
-- this event is called every time the engine receives a keypress
-- for non-toggleable keys this event may be fired multiple times
-- always check whether a key was unpressed when necessary
-- 
onKeyDown = function(event)
	-- Stop engine if ESC is pressed
	-- 256 is the keycode of the ESC key
	--
	if event.keycode == 256 then
		event.engine:Stop()
		
		return 0
	end

	local scene_manager = event.engine:GetSceneManager()

	local moveSpeed = 25.0 * event.deltaTime;
	  
	local camera_h, camera_v = scene_manager:GetCameraHVRotation();

	local pitch = math.rad(camera_v)
	local yaw = math.rad(camera_h) 

	local front_x = math.cos(yaw) * math.cos(pitch);
	local front_y = math.sin(pitch);
	local front_z = math.sin(yaw) * math.cos(pitch);

	-- world_up = 0, 1, 0
	local right_x, right_y, right_z = vectorCross(front_x, front_y, front_z, 0, 1, 0)

	local up_x, up_y, up_z = vectorCross(right_x, right_y, right_z, front_x, front_y, front_z)

	local transform_x, transform_y, transform_z = scene_manager:GetCameraTransform();
	
	local keycodeSwitchTbl = {
	   [string.byte('W')] = function()
		  transform_x = transform_x + front_x * moveSpeed;
		  transform_y = transform_y + front_y * moveSpeed;
		  transform_z = transform_z + front_z * moveSpeed;
	   end,
	   [string.byte('S')] = function()
		  transform_x = transform_x - front_x * moveSpeed;
		  transform_y = transform_y - front_y * moveSpeed;
		  transform_z = transform_z - front_z * moveSpeed;
	   end,
	   [string.byte('A')] = function()
		  transform_x = transform_x - right_x * moveSpeed;
		  transform_y = transform_y - right_y * moveSpeed;
		  transform_z = transform_z - right_z * moveSpeed;
	   end,
	   [string.byte('D')] = function()
		  transform_x = transform_x + right_x * moveSpeed;
		  transform_y = transform_y + right_y * moveSpeed;
		  transform_z = transform_z + right_z * moveSpeed;
	   end,
	   [340] = function() -- shift key is value 340
		  transform_x = transform_x - up_x * moveSpeed;
		  transform_y = transform_y - up_y * moveSpeed;
		  transform_z = transform_z - up_z * moveSpeed;
	   end
	};
	
	if keycodeSwitchTbl[event.keycode] ~= nil then
	   keycodeSwitchTbl[event.keycode]();
	   
	   scene_manager:SetCameraTransform(transform_x, transform_y, transform_z);
	end

	return 0
end

-- ON_KEYUP event
-- 
-- this event is called exactly once for every release of a key
-- 
onKeyUp = function(event)
	return 0
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
	scene_manager:SetCameraTransform(-2.0, 55.8, 4.7)
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

	for chunk_x = -chunks_x, chunks_x, 1 do -- chunks_x, chunks_x, 1
		chunks[chunk_x] = {}
		for chunk_z = -chunks_z, chunks_z, 1 do
			local chunk_obj = engine.CreateSceneObject(asset_manager, "renderer_3d/generator", "terrain", {
				{"texture", "atlas"}, {"generator_script", "testgen"}, {"generator_supplier", "script0/terrain_generator"}
			})
			chunk_obj:SetPosition((chunk_x) * config.chunk_size_x, 0.0, (chunk_z) * config.chunk_size_z)
			chunk_obj:SetSize(1, 1, 1)
			chunk_obj:SetRotation(0, 0, 0)
			scene:AddObject(string.format("chunk_%i_%i", chunk_x, chunk_z), chunk_obj)
			chunks[chunk_x][chunk_z] = chunk_obj
			--print(string.format("Building chunk [%i,%i]",chunk_x, chunk_z))
			--engine.RendererForceBuild(chunk_obj.renderer)
		end
	end

	print("Hi!")

	return 0
end
