-- Include modules

-- Debugging data
local deltaTime_accum = 0.0
local deltaTime_count = 0
-- Minimum & Maximum reached deltatime
local deltaTime_max = 0.0
local deltaTime_min = 2000.0

-----------------------
--->  Game events  <---


-- ON_MOUSEMOVED event
-- 
-- this event is called when the mouse moved
--
-- while specifying event handlers is optional
-- it is left here for illustration purposes
-- (events for which no event handler is specified are discarded)
-- 
onMouseMoved = function(event)
	return 0
end

-- ON_KEYDOWN event
-- 
-- this event is called every time the engine receives a keypress
-- for non-toggleable keys this event may be fired multiple times
-- always check whether a key was unpressed when necessary
-- 
onKeyDown = function(event)
	local scene_manager = event.engine:getSceneManager();

	-- Check for space press
	--
	-- note: it is not recommended to convert event.keycode with string.char
	--       as it may fail when encoutering a non-character value
	--
	-- note: event.keycode is a 16-bit value
	--       ASCII characters match keycode if converted with string.byte
	--       for function key keycodes search for "GLFW key tokens"
	--
	if event.keycode == string.byte(' ') then
		scene_manager:setScene("floppygame")
	end

	-- Stop engine if ESC is pressed
	-- 256 is the keycode of the ESC key
	--
	if event.keycode == 256 then
		event.engine:stop()
	end

	return 0
end

-- ON_KEYUP event
-- 
-- this event is called exactly once for every release of a key
-- 
onKeyUp = function(event)
	-- event.keycode is the same for ON_KEYUP and ON_KEYDOWN events
	--
	if event.keycode == string.byte(' ') then
		birb_is_velociting = false
	end
end

-- Minimum & Maximum reached deltatime
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

	local asset_manager = event.engine:getAssetManager()
	local scene_manager = event.engine:getSceneManager()
	local scene = scene_manager:getSceneCurrent()

	-- Updates frametime counter, recommend to leave this here for debugging purposes
	if deltaTime_accum >= 1.0 then
		local deltaTime_avg = deltaTime_accum / deltaTime_count
		--print(string.format("Frametime is: %f ms!", deltaTime_accum / deltaTime_count * 1000))
		local object = scene:findObject("_debug_info")
		meshBuilderSupplyData(object.meshbuilder, "text", string.format("avg: %fms\nmin: %fms\nmax: %fms", deltaTime_avg * 1000, deltaTime_min * 1000, deltaTime_max * 1000))

		deltaTime_min = 2000.0
		deltaTime_max = 0.0
		deltaTime_accum = 0
		deltaTime_count = 0
	end

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

	print("Default scene ON_INIT")

	-- Create frametime counter and add it to scene
	local object = createSceneObject(event.engine, "renderer_2d/text", "text",
		{ {"font", font} }, { {"text", "avg: \nmin: \nmax: "} }
	)
	object:setPosition(0.0, 0.0, -0.01)
	object:setSize(24, 24, 1.0)
	scene:addObject("_debug_info", object)

	-- Add score
	local object = createSceneObject(event.engine, "renderer_2d/text", "text", {
		{"font", font}
	}, {{"text", "Press space to begin"}})
	object:setPosition(0.3, 0.5, -0.01)
	object:setSize(48, 48, 1.0)
	scene:addObject("text_begin", object)

	-- Set-up camera
	-- (this is essentially unnecessary for 2D-only scenes)
	scene_manager:setCameraTransform(0.0, 0.0, 0.0)
	scene_manager:setCameraRotation(0, 0)

	-- Set-up light
	-- (unnecessary for scenes that don't employ renderers or shaders with lighting)
	scene_manager:setLightTransform(-1, 1, 0)

	return 0
end

--->  Game events  <---
-----------------------
