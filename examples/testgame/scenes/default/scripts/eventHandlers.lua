-- ON_MOUSEMOVED event
-- 
-- this event is called when the mouse moved
--
-- while specifying event handlers is optional
-- it is left here for illustration purposes
-- (events for which no event handler is specified are discarded)
-- 
onMouseMoved = function(event)
	local mouseSpeed = 0.05;

	local scene_manager = event.engine:GetSceneManager()

	local h, v = scene_manager:GetCameraHVRotation()

	--engine.logger.SimpleLog(string.format("X: %f Y: %f", event.mouse.x, event.mouse.y))

	h = h + (mouseSpeed * event.deltaTime) * event.mouse.x
	v = v + (mouseSpeed * event.deltaTime) * event.mouse.y

	scene_manager:SetCameraHVRotation(h, v)

	return 0
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

	local moveSpeed = 5.0 * event.deltaTime;
      
	local camera_h, camera_v = scene_manager:GetCameraHVRotation();

	local direction_x = math.cos(camera_v) * math.cos(camera_h);
	local direction_y = math.sin(camera_v);
	local direction_z = math.cos(camera_v) * math.sin(camera_h);

	--{x = v1.y*v2.z - v2.y*v1.z , y = v2.x*v1.z-v1.x*v2.z , z = v1.x*v2.y-v2.x*v1.y}
	local right_x = direction_x * 0.0 - 1.0 * direction_z;
	local right_y = 0.0 * direction_z - direction_x * 0.0;
	local right_z = direction_x * 1.0 - 0.0 * direction_y;
	
	local transform_x, transform_y, transform_z = scene_manager:GetCameraTransform();
	
	local keycodeSwitchTbl = {
	   [string.byte('W')] = function()
		  transform_x = transform_x + direction_x * moveSpeed;
		  transform_y = transform_y + direction_y * moveSpeed;
		  transform_z = transform_z + direction_z * moveSpeed;
	   end,
	   [string.byte('S')] = function()
		  transform_x = transform_x - direction_x * moveSpeed;
		  transform_y = transform_y - direction_y * moveSpeed;
		  transform_z = transform_z - direction_z * moveSpeed;
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

-- Debugging data
local deltaTime_accum = 0.0
local deltaTime_count = 0
local deltaTime_max = 0.0
local deltaTime_min = 2000.0

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

	local asset_manager = event.engine:GetAssetManager()
	local scene_manager = event.engine:GetSceneManager()
	local scene = scene_manager:GetSceneCurrent()

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

	-- Create frametime counter and add it to scene
	local object = engine.CreateSceneObject(asset_manager, "renderer_2d/text", "text", {
		{"font", "myfont"}, {"text", "avg: \nmin: \nmax: "}
	  })
	object:SetPosition(0.0, 0.0, -0.01)
	object:SetSize(24, 24, 1.0)
	scene:AddObject("_debug_info", object)
	
	local object = engine.CreateSceneObject(asset_manager, "renderer_2d/text", "text", {
		{"font", "myfont"}, {"text", "H: V: \nX: Y: Z: "}
	  })
	object:SetPosition(0.6, 0.0, -0.01)
	object:SetSize(24, 24, 1.0)
	scene:AddObject("_debug_info2", object)

	local object = engine.CreateSceneObject(asset_manager, "renderer_3d/text", "text", {
		{"font", "myfont"}, {"text", "TEST TEXT"}
	})
	object:SetPosition(0.5, 0.3, 0.0)
	object:SetSize(128, 128, 1.0)
	object:SetRotation(0, -135, 180)
	scene:AddObject("test3d", object)

	-- Set-up camera
	scene_manager:SetCameraTransform(-3.0, 2.3, 4.7)
	scene_manager:SetCameraHVRotation(2.0, 3.4)

	return 0
end