-- Include modules
local util = require("util")
local config = require("config")

-----------------------
---->  Game data  <----

-- Debugging data
local deltaTime_accum = 0.0
local deltaTime_count = 0
-- Minimum & Maximum reached deltatime
local deltaTime_max = 0.0
local deltaTime_min = 2000.0

-- Related to spawning pipes
local spawn_pipe_every = config.spawn_pipe_every_start
local spawn_pipe_last_idx = 0
local spawn_pipe_first_idx = 1
local spawn_pipe_count = 0
local spawn_pipe_since_last = spawn_pipe_every - 0.1

-- Pipe behavior
local pipe_spacing_start = 200 -- Configurable pipe spacing (between top and bottom)
local pipe_spacing = pipe_spacing_start
local pipe_move_speed = config.pipe_move_speed

-- Birb behavior
local birb_jump_velocity = 0.46 -- Configurable velocity set on jump
local birb_gravity = 0.89 -- Configurable gravity
local birb_velocity = 0.25 -- Internal (vertical) velocity value
local birb_is_velociting = false -- Internal is-jumping check

-- Game state
local game_gameover_state = false -- After falling animation, complete game end
local game_midgameover_state = false -- After collision but before end of falling animation
local game_last_scored_pipe_idx = 0 -- Index of last scored pipe
local game_score = 0 -- The score

---->  Game data  <----
-----------------------

-----------------------
--> Local functions <--

-- Scrolls ground objects
local handleGroundLayer = function(scene, event, layer)

	local ground3 = scene:findObject(layer..2)
	local g3_x, g3_y, g3_z = ground3:getPosition()
	g3_x = g3_x - (pipe_move_speed * 1.1) * event.deltaTime
	
	local last_x = g3_x

	for i = 0, 2 do
		local ground = scene:findObject(layer..i)
		local ground_x, ground_y, ground_z = ground:getPosition()

		-- Move ground (slightly faster than pipes)
		ground_x = ground_x - (pipe_move_speed * 1.1) * event.deltaTime
	
		if(ground_x > util.pxToScreenX(-630))
		then
			ground:setPosition(ground_x, ground_y, ground_z)
		else
			ground:setPosition(last_x + util.pxToScreenX(630), ground_y, ground_z)
		end

		last_x = ground_x
	end
end

local gameOnHandleGrounds = function(scene, event)
	handleGroundLayer(scene, event, "ground_top")
	handleGroundLayer(scene, event, "ground_bottom")
end

local checkCollisionsGrounds = function(birbx, birby)
	return (util.checkCollisions2DBox(birbx, 	birby,								util.pxToScreenX(config.birb_x_size),	util.pxToScreenY(config.birb_y_size),
									  0.0,		0.0,								1.0,						util.pxToScreenY(60)) or
			util.checkCollisions2DBox(birbx, 	birby,								util.pxToScreenX(config.birb_x_size), 	util.pxToScreenY(config.birb_y_size),
								 	  0.0,		util.pxToScreenY(util.screen_size_y - 60), 	1.0, 					 	util.pxToScreenY(60)))
end

local gameOnGameOver = function(engine, asset_manager, scene_manager)
	game_midgameover_state = true
	birb_velocity = -0.4

	print("Game over!")

	local font = asset_manager:getFont("myfont")

	local object = createSceneObject(engine, "renderer_2d/text", "text", {
		{"font", font}}, { {"text", "GAME OVER"} }
	)
	object:setPosition(0.34, 0.45, -0.01)
	object:setSize(64, 64, 1.0)
	scene_manager:getSceneCurrent():addObject("text_gameover", object)
end

--> Local functions <--
-----------------------

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
	-- Check for space press
	--
	-- note: it is not recommended to convert event.keycode with string.char
	--       as it may fail when encoutering a non-character value
	--
	-- note: event.keycode is a 16-bit value
	--       ASCII characters match keycode if converted with string.byte
	--       for function key keycodes search for "GLFW key tokens"
	--
	if event.keycode == string.byte(' ') and
	   game_midgameover_state == false
	then
		if birb_is_velociting == false then
			birb_velocity = birb_jump_velocity
			birb_is_velociting = true
			return 0
		end
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

	-- For profiling only!
	if (game_score >= 250) then
		event.engine:Stop()
	end

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

	if (game_gameover_state == false and game_midgameover_state == false) then
		-- Spawn new pipes
		if spawn_pipe_since_last > spawn_pipe_every then
			-- Constant offset to center the pipes
			local const_pipe_y_offset = -80
			-- We can use the math library in here!
			local pipe_y_offset = const_pipe_y_offset + math.random(-15, 15) * 10
			--local pipe_y_offset = const_pipe_y_offset
			
			local pipe_id = tostring(spawn_pipe_last_idx + 1)

			local pipe1 = scene:addTemplatedObject("sprite_pipe_down"..pipe_id, "pipe_down")
			pipe1:setPosition(1.0, util.pxToScreenY(pipe_y_offset - (pipe_spacing / 2)), -0.15)
			pipe1:setSize(util.pxToScreenX(config.pipe_x_size), util.pxToScreenY(config.pipe_y_size), 1.0)

			local pipe2 = scene:addTemplatedObject("sprite_pipe_up"..pipe_id, "pipe_up")
			pipe2:setPosition(1.0, util.pxToScreenY(config.pipe_y_size + (pipe_spacing / 2) + pipe_y_offset), -0.15)
			pipe2:setSize(util.pxToScreenX(config.pipe_x_size), util.pxToScreenY(config.pipe_y_size), 1.0)

			spawn_pipe_last_idx = spawn_pipe_last_idx + 1
			spawn_pipe_count = spawn_pipe_count + 1
			spawn_pipe_since_last = 0.0
		end

		spawn_pipe_since_last = spawn_pipe_since_last + event.deltaTime

		-- Get birb position
		local birb = scene:findObject("birb")
		local birbx, birby, birbz = birb:getPosition()

		if spawn_pipe_count >= 1 then
			for pipeIdx = spawn_pipe_first_idx, spawn_pipe_last_idx, 1 do
				-- Move pipes
				local pipe1 = scene:findObject("sprite_pipe_down"..tostring(pipeIdx))
				local pipe2 = scene:findObject("sprite_pipe_up"..tostring(pipeIdx))
				local x1, y1, z1 = pipe1:getPosition()
				local x2, y2, z2 = pipe2:getPosition()
				x1 = x1 - pipe_move_speed * event.deltaTime
				x2 = x2 - pipe_move_speed * event.deltaTime
				pipe1:setPosition(x1, y1, z1)
				pipe2:setPosition(x2, y2, z2)

				-- Check collisions with both pipes
				if util.checkCollisions2DBox(birbx, birby, util.pxToScreenX(config.birb_x_size), util.pxToScreenY(config.birb_y_size), x1, y1, util.pxToScreenX(config.pipe_x_size), util.pxToScreenY(config.pipe_y_size)) or -- pipe 1
				   util.checkCollisions2DBox(birbx, birby, util.pxToScreenX(config.birb_x_size), util.pxToScreenY(config.birb_y_size), x2, y2, util.pxToScreenX(config.pipe_x_size), util.pxToScreenY(config.pipe_y_size))	 -- pipe 2
				then
					gameOnGameOver(event.engine, asset_manager, scene_manager)
					return 0
				end

				-- Add score by colliding with a wall after the pipes
				if util.checkCollisions2DBox(birbx, birby, util.pxToScreenX(config.birb_x_size), util.pxToScreenY(config.birb_y_size), x2 + util.pxToScreenX(80), 0.0, util.pxToScreenX(80), 1.0) and
				   pipeIdx > game_last_scored_pipe_idx
				then
					game_score = game_score + 1
					game_last_scored_pipe_idx = pipeIdx
					local score_object = scene:findObject("text_score")
					meshBuilderSupplyData(score_object.meshbuilder, "text", tostring(game_score))

					pipe_move_speed = pipe_move_speed * 1.005 -- Increase pipe move speed
					pipe_spacing = pipe_spacing * 0.9990 -- Decrease pipe spacing (between top and bottom pipe)
					spawn_pipe_every = spawn_pipe_every * 0.9950 -- Increase spawn rate (decrease timeout between spawns)
				end

				if x1 < (0.0 - util.pxToScreenX(config.pipe_x_size + 5)) then
					-- Destroy objects
					scene:removeObject("sprite_pipe_down"..tostring(pipeIdx))
					scene:removeObject("sprite_pipe_up"..tostring(pipeIdx))
					spawn_pipe_first_idx = spawn_pipe_first_idx + 1
				end
			end
		end
		
		-- Check collisions with grounds
		if checkCollisionsGrounds(birbx, birby)
		then
			gameOnGameOver(event.engine, asset_manager, scene_manager)
			return 0
		end

		-- Fall birb
		-- we already have birbx/y/z from before
		birby = birby - birb_velocity * event.deltaTime
		birb:setPosition(birbx, birby, birbz)
		birb_velocity = birb_velocity - config.birb_gravity * event.deltaTime

		gameOnHandleGrounds(scene, event)

	elseif (game_midgameover_state == true and game_gameover_state == false) then

		local birb = scene:findObject("birb")
		local birbx, birby, birbz = birb:getPosition()

		birby = birby - birb_velocity * event.deltaTime
		birb:setPosition(birbx, birby, birbz)
		
		birb_velocity = birb_velocity * 1.01

		-- Check collisions with grounds
		if checkCollisionsGrounds(birbx, birby)
		then
			game_gameover_state = true
		end
	end

	return 0
end

-- ON_INIT event
--
-- this event is fired after the scene is loaded and before the first rendered frame
-- perform initialization here
--
onInit = function(event)
	-- By default the framerate target is 0 (VSYNC)
	-- uncomment this to set the desired framerate target
	-- the engine will spinsleep until the target is reached
	--
	--event.engine:setFramerateTarget(30)

	-- Get managers
	local asset_manager = event.engine:getAssetManager()
	local scene_manager = event.engine:getSceneManager()
	local scene = scene_manager:getSceneCurrent()

	local font = asset_manager:getFont("myfont")

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
	}, {{"text", "0"}})
	object:setPosition(0.5, 0.0, -0.01)
	object:setSize(64, 64, 1.0)
	scene:addObject("text_score", object)

	-- Add background
	local object = createSceneObject(event.engine, "renderer_2d/sprite", "sprite", {
		{"texture", asset_manager:getTexture("background")}
	}, {})
	object:setPosition(0.0, 0.0, -0.8)
	object:setSize(1, 1, 1)
	scene:addObject("background", object)

	-- Add birb
	local object = createSceneObject(event.engine, "renderer_2d/sprite", "sprite", {
		{"texture", asset_manager:getTexture("birb")}
	}, {})
	object:setPosition(0.2, util.pxToScreenY(360), 0.0)
	object:setSize(util.pxToScreenX(72), util.pxToScreenY(44), 1.0)
	scene:addObject("birb", object)

	-- Add grounds
	for i = 0, 2 do
		local ground_top = createSceneObject(event.engine, "renderer_2d/sprite", "sprite", {
			{"texture", asset_manager:getTexture("ground_top")}
		}, {})
		ground_top:setPosition(util.pxToScreenX(630) * i, 0.0, -0.1)
		ground_top:setSize(util.pxToScreenX(630), util.pxToScreenY(60), 1)
		scene:addObject("ground_top"..i, ground_top)

		local ground_bottom = createSceneObject(event.engine, "renderer_2d/sprite", "sprite", {
			{"texture", asset_manager:getTexture("ground_bottom")}
	 	}, {})

		ground_bottom:setPosition(util.pxToScreenX(630) * i, util.pxToScreenY(util.screen_size_y - 60), -0.1)
		ground_bottom:setSize(util.pxToScreenX(630), util.pxToScreenY(60), 1)
		scene:addObject("ground_bottom"..i, ground_bottom)
	end

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
