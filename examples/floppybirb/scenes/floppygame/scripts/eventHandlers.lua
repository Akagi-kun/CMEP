-----------------------
---->  Game data  <----

local deltaTime_accum = 0.0;
local deltaTime_count = 0;

-- Related to spawning pipes
local spawn_pipe_every = 4.5;
local spawn_pipe_since_last = spawn_pipe_every - 0.1;
local spawn_pipe_last_idx = 0;
local spawn_pipe_first_idx = 1;
local spawn_pipe_count = 0;

-- Related to pipes themselves (note: pipe original size is 110x338)
local pipe_x_size = 110; -- was <const>
local pipe_y_size = 450; -- was <const>

-- Related to pipe behavior
local pipe_spacing_start = 200; -- was <const>
local pipe_spacing = pipe_spacing_start;
local pipe_move_speed = 0.12;

-- Related to birb
local birb_x_size = 72; -- was <const>
local birb_y_size = 44; -- was <const>
local birb_jump_velocity = 0.36; -- was <const>
local birb_gravity = 0.68; -- was <const>
local birb_velocity = 0.1;
local birb_is_velociting = false;

-- Game state
local game_gameover_state = false;
local game_last_scored_pipe_idx = 0;
local game_score = 0;

---->  Game data  <----
-----------------------

-----------------------
--> Local functions <--

local checkCollisions2DBox = function(x1, y1, w1, h1, x2, y2, w2, h2)
	if (
	 x1 < x2 + w2 and
	 x1 + w1 > x2 and
	 y1 < y2 + h2 and
	 y1 + h1 > y2
	) then
		return true;
	else
		return false
	end
end

local screen_size_x = 1100;
local screen_size_y = 720;

local pxToScreenX = function(x)
	return x / screen_size_x
end

local pxToScreenY = function(y)
	return y / screen_size_y
end

local gameOnGameOver = function(asset_manager, scene_manager)
	cmepmeta.logger.SimpleLog(string.format("Game over!"))

	local font = asset_manager:GetFont("myfont");
	local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.34, 0.45, -0.01, 64, "GAME OVER", font);
	scene_manager:AddObject("text_gameover", object);
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
	return 0;
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
		event.engine:Stop();
		return 0;
	end

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
		if birb_is_velociting == false then
			birb_velocity = birb_jump_velocity;
			birb_is_velociting = true;
		end
	end

	return 0;
end

-- ON_KEYUP event
-- 
-- this event is called exactly once for every release of a key
-- 
onKeyUp = function(event)
	-- event.keycode is the same for ON_KEYUP and ON_KEYDOWN events
	--
	if event.keycode == string.byte(' ') then
		birb_is_velociting = false;
	end
end

max_deltatime_avg = 0.0
min_deltatime_avg = 1000.0
--was_logged = false

-- ON_UPDATE event
-- 
-- called every frame
--
-- event.deltaTime (float) contains the delta time in microseconds
-- and is the period between the last onUpdate and the current one
--
onUpdate = function(event)
	deltaTime_accum = deltaTime_accum + event.deltaTime;

	max_deltatime_avg = math.max(max_deltatime_avg, event.deltaTime);
	min_deltatime_avg = math.min(min_deltatime_avg, event.deltaTime);

	deltaTime_count = deltaTime_count + 1;

	local asset_manager = event.engine:GetAssetManager();
	local scene_manager = event.engine:GetSceneManager();

	-- Updates frametime counter, recommend to leave this here for debugging purposes
	if deltaTime_accum >= 1.0 then
		local deltaTime_avg = deltaTime_accum / deltaTime_count
		--cmepmeta.logger.SimpleLog(string.format("Hello from Lua! Last FT is: %f ms!", deltaTime_accum / deltaTime_count * 1000))
		local object = scene_manager:FindObject("_debug_info");
		cmepapi.TextRendererUpdateText(object.renderer, string.format("avg: %fms\nmin: %fms\nmax: %fms", deltaTime_avg * 1000, min_deltatime_avg * 1000, max_deltatime_avg * 1000));
		
		min_deltatime_avg = 1000.0
		max_deltatime_avg = 0.0
		deltaTime_accum = 0;
		deltaTime_count = 0;
	end

	if game_gameover_state == false then
		if spawn_pipe_since_last > spawn_pipe_every then
			-- Spawn new pipes

			-- We can use the math library in here!
			local const_pipe_y_offset = -100
			--local pipe_y_offset = const_pipe_y_offset
			local pipe_y_offset = const_pipe_y_offset + math.random(-15, 15) * 10;

			scene_manager:AddTemplatedObject("sprite_pipe_down"..tostring(spawn_pipe_last_idx + 1), "pipe_down");
			local pipe1 = scene_manager:FindObject("sprite_pipe_down"..tostring(spawn_pipe_last_idx + 1));
			pipe1:Translate(1.0, pxToScreenY(pipe_y_offset - (pipe_spacing / 2)), -0.15);
			pipe1:Scale(pxToScreenX(pipe_x_size), pxToScreenY(pipe_y_size), 1.0);

			scene_manager:AddTemplatedObject("sprite_pipe_up"..tostring(spawn_pipe_last_idx + 1), "pipe_up");
			local pipe2 = scene_manager:FindObject("sprite_pipe_up"..tostring(spawn_pipe_last_idx + 1));
			pipe2:Translate(1.0, pxToScreenY(pipe_y_size + (pipe_spacing / 2) + pipe_y_offset), -0.15);
			pipe2:Scale(pxToScreenX(pipe_x_size), pxToScreenY(pipe_y_size), 1.0);

			spawn_pipe_last_idx = spawn_pipe_last_idx + 1;
			spawn_pipe_count = spawn_pipe_count + 1;
			spawn_pipe_since_last = 0.0;
		end

		-- Get birb position
		local birb = scene_manager:FindObject("birb");
		local birbx, birby, birbz = birb:GetPosition();

		if spawn_pipe_count >= 1 then
			for pipeIdx = spawn_pipe_first_idx, spawn_pipe_last_idx, 1 do
				-- Move pipes
				local pipe1 = scene_manager:FindObject("sprite_pipe_down"..tostring(pipeIdx));
				local pipe2 = scene_manager:FindObject("sprite_pipe_up"..tostring(pipeIdx));
				local x1, y1, z1 = pipe1:GetPosition();
				local x2, y2, z2 = pipe2:GetPosition();
				x1 = x1 - pipe_move_speed * event.deltaTime;
				x2 = x2 - pipe_move_speed * event.deltaTime;
				pipe1:Translate(x1, y1, z1);
				pipe2:Translate(x2, y2, z2);

				-- Check collisions with both pipes
				if checkCollisions2DBox(birbx, birby, pxToScreenX(birb_x_size), pxToScreenY(birb_y_size), x1, y1, pxToScreenX(pipe_x_size), pxToScreenY(pipe_y_size)) or -- pipe 1
				   checkCollisions2DBox(birbx, birby, pxToScreenX(birb_x_size), pxToScreenY(birb_y_size), x2, y2, pxToScreenX(pipe_x_size), pxToScreenY(pipe_y_size))    -- pipe 2
				then
					game_gameover_state = true;

					gameOnGameOver(asset_manager, scene_manager)
					return 0;
				end

				-- Add score by colliding with a wall after the pipes
				if checkCollisions2DBox(birbx, birby, pxToScreenX(birb_x_size), pxToScreenY(birb_y_size), x2 + pxToScreenX(80), 0.0, pxToScreenX(80), 1.0) and
				   pipeIdx > game_last_scored_pipe_idx
				then
					game_score = game_score + 1;
					game_last_scored_pipe_idx = pipeIdx;
					local score_object = scene_manager:FindObject("text_score");
					cmepapi.TextRendererUpdateText(score_object.renderer, tostring(game_score));

					pipe_move_speed = pipe_move_speed * 1.005; -- Increase pipe move speed
					pipe_spacing = pipe_spacing * 0.9990; -- Decrease pipe spacing (between top and bottom pipe)
					spawn_pipe_every = spawn_pipe_every * 0.9950; -- Increase spawn rate (decrease timeout between spawns)
				end

				if x1 < (0.0 - pxToScreenX(pipe_x_size + 5)) then
					-- Destroy objects
					scene_manager:RemoveObject("sprite_pipe_down"..tostring(pipeIdx));
					scene_manager:RemoveObject("sprite_pipe_up"..tostring(pipeIdx));
					spawn_pipe_first_idx = spawn_pipe_first_idx + 1;
				end
			end
		end
		
		-- Check collisions with grounds
		if 	checkCollisions2DBox(birbx, 	birby,								pxToScreenX(birb_x_size),	pxToScreenY(birb_y_size),
								 0.0,		 0.0,								1.0,						pxToScreenY(60)) or
			checkCollisions2DBox(birbx, 	birby,								pxToScreenX(birb_x_size), 	pxToScreenY(birb_y_size),
								  0.0,		pxToScreenY(screen_size_y - 60), 	1.0, 					 	pxToScreenY(60))
		then
			game_gameover_state = true;

			gameOnGameOver(asset_manager, scene_manager)
			return 0;
		end

		-- Fall birb
		-- we already have birbx/y/z from before
		birby = birby - birb_velocity * event.deltaTime;
		birb:Translate(birbx, birby, birbz);

		local ground1 = scene_manager:FindObject("ground_top");
		local ground2 = scene_manager:FindObject("ground_bottom");
		local g1_x, g1_y, g1_z = ground1:GetPosition();
		local g2_x, g2_y, g2_z = ground2:GetPosition();
		
		-- Move ground on X axis (slightly faster than pipes)
		g1_x = g1_x - (pipe_move_speed * 1.1) * event.deltaTime;
		g2_x = g2_x - (pipe_move_speed * 1.1) * event.deltaTime;

		-- Check whether ground is in a correct position
		-- (the right end of sprite cannot be visible)
		if	(g1_x > pxToScreenX(-120)) or
			(g2_x > pxToScreenX(-120))
		then
			ground1:Translate(g1_x, g1_y, g1_z);
			ground2:Translate(g2_x, g2_y, g2_z);
		-- If it's not, reset back to 0
		else
			ground1:Translate(0.0, g1_y, g1_z);
			ground2:Translate(0.0, g2_y, g2_z);
		end

		-- Add birb_gravity to birb velocity
		birb_velocity = birb_velocity - birb_gravity * event.deltaTime;
		spawn_pipe_since_last = spawn_pipe_since_last + event.deltaTime;
	end

	return 0;
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
	--event.engine:SetFramerateTarget(60);

	-- Get managers
	local asset_manager = event.engine:GetAssetManager();
	local scene_manager = event.engine:GetSceneManager();

	-- Create frametime counter and add it to scene
	local font = asset_manager:GetFont("myfont");
	local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.0, 0.0, -0.01, 24, "avg: \nmin: \nmax: ", font);
	scene_manager:AddObject("_debug_info", object);
	
	-- Add score
	local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.5, 0.0, -0.01, 64, "0", font);
	scene_manager:AddObject("text_score", object);

	-- Set-up camera
	-- (this is essentially unnecessary for 2D-only scenes)
	scene_manager:SetCameraTransform(0.0, 0.0, 0.0);
	scene_manager:SetCameraHVRotation(0, 0);

	-- Set-up light
	-- (unnecessary for scenes that don't employ renderers which support lighting)
	scene_manager:SetLightTransform(-1, 1, 0);

	return 0;
end

--->  Game events  <---
-----------------------
