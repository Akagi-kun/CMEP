-----------------------
---->  Game data  <----

local deltaTimeAvg = 0.0;
local deltaTimeCount = 0;

local spawnPipeEvery = 4.5;
local spawnPipeSinceLast = spawnPipeEvery - 0.1;
local spawnPipeLastIdx = 0;
local spawnPipeFirstIdx = 1;
local spawnPipeCount = 0;
local pipeMoveSpeed = 0.12;

-- Pipes original size is 110x338
local pipe_xSize = 110; -- was <const>
local pipe_ySize = 450; -- was <const>
local pipe_spacing_start = 200; -- was <const>
local pipe_spacing = pipe_spacing_start;

local birb_xSize = 72; -- was <const>
local birb_ySize = 44; -- was <const>

local gameIsGameOver = false;
local gameLastScoredPipeIdx = 0;
local gameScore = 0;

local birbSetVelocityTo = 0.36; -- was <const>
local birbFallSpeed = 0.68; -- was <const>

birbVelocity = 0.1;
birbIsVelociting = false;

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

local screenX = 1100;
local screenY = 720;

local pxToScreenX = function(x)
	return x / screenX
end

local pxToScreenY = function(y)
	return y / screenY
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
		if birbIsVelociting == false then
			birbVelocity = birbSetVelocityTo;
			birbIsVelociting = true;
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
		birbIsVelociting = false;
	end
end

-- ON_UPDATE event
onUpdate = function(event)
	deltaTimeAvg = deltaTimeAvg + event.deltaTime;
	deltaTimeCount = deltaTimeCount + 1;

	local asset_manager = event.engine:GetAssetManager();
	local scene_manager = event.engine:GetSceneManager();
	--local asset_manager = cmepapi.EngineGetAssetManager(event.engine);
	--local scene_manager = cmepapi.EngineGetSceneManager(event.engine);

	-- Update frametime counter, recommend to leave this here for debugging purposes
	if deltaTimeCount >= 30 then
		--cmepmeta.logger.SimpleLog(string.format("Hello from Lua! Last FT is: %f ms!", deltaTimeAvg / deltaTimeCount * 1000))
		local object = scene_manager:FindObject("_debug_info");
		cmepapi.TextRendererUpdateText(object.renderer, "FT: "..tostring(deltaTimeAvg / deltaTimeCount * 1000).." ms");
		
		deltaTimeAvg = 0;
		deltaTimeCount = 0;
	end

	-- We can use the math library in here!
	local offset = -200 + math.random(-15, 15) * 10;

	if gameIsGameOver == false then
		if spawnPipeSinceLast > spawnPipeEvery then
			-- Spawn new pipes

			scene_manager:AddTemplatedObject("sprite_pipe_down"..tostring(spawnPipeLastIdx + 1), "pipe_down");
			local object1 = scene_manager:FindObject("sprite_pipe_down"..tostring(spawnPipeLastIdx + 1));
			object1:Translate(1.0, pxToScreenY(offset), -0.15);
			object1:Scale(pxToScreenX(pipe_xSize), pxToScreenY(pipe_ySize), 1.0);

			scene_manager:AddTemplatedObject("sprite_pipe_up"..tostring(spawnPipeLastIdx + 1), "pipe_up");
			local object2 = scene_manager:FindObject("sprite_pipe_up"..tostring(spawnPipeLastIdx + 1));
			object2:Translate(1.0, pxToScreenY(pipe_ySize + pipe_spacing + offset), -0.15);
			object2:Scale(pxToScreenX(pipe_xSize), pxToScreenY(pipe_ySize), 1.0);

			spawnPipeLastIdx = spawnPipeLastIdx + 1;
			spawnPipeCount = spawnPipeCount + 1;
			spawnPipeSinceLast = 0.0;
		end

		-- Get birb position
		local birb = scene_manager:FindObject("birb");
		local birbx, birby, birbz = birb:GetPosition();

		if spawnPipeCount >= 1 then
			for pipeIdx = spawnPipeFirstIdx, spawnPipeLastIdx, 1 do
				-- Move pipes
				local pipe1 = scene_manager:FindObject("sprite_pipe_down"..tostring(pipeIdx));
				local pipe2 = scene_manager:FindObject("sprite_pipe_up"..tostring(pipeIdx));
				local x1, y1, z1 = pipe1:GetPosition();
				local x2, y2, z2 = pipe2:GetPosition();
				x1 = x1 - pipeMoveSpeed * event.deltaTime;
				x2 = x2 - pipeMoveSpeed * event.deltaTime;
				pipe1:Translate(x1, y1, z1);
				pipe2:Translate(x2, y2, z2);

				-- Check collisions with both pipes
				if checkCollisions2DBox(birbx, birby, pxToScreenX(birb_xSize), pxToScreenY(birb_ySize), x1, y1, pxToScreenX(pipe_xSize), pxToScreenY(pipe_ySize)) or -- pipe 1
				   checkCollisions2DBox(birbx, birby, pxToScreenX(birb_xSize), pxToScreenY(birb_ySize), x2, y2, pxToScreenX(pipe_xSize), pxToScreenY(pipe_ySize))    -- pipe 2
				then
					gameIsGameOver = true;
					local font = asset_manager:GetFont("myfont");
					local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.4, 0.4, -0.01, 32, "GAME OVER", font);
					scene_manager:AddObject("text_gameover", object);
					return 0;
				end

				-- Add score by colliding with a wall after the pipes
				if checkCollisions2DBox(birbx, birby, pxToScreenX(birb_xSize), pxToScreenY(birb_ySize), x2 + pxToScreenX(80), 0.0, pxToScreenX(80), 1.0) and
				   pipeIdx > gameLastScoredPipeIdx
				then
					gameScore = gameScore + 1;
					gameLastScoredPipeIdx = pipeIdx;
					local score_object = scene_manager:FindObject("text_score");
					cmepapi.TextRendererUpdateText(score_object.renderer, tostring(gameScore));

					pipeMoveSpeed = pipeMoveSpeed * 1.01;
					pipe_spacing = pipe_spacing * 0.990;
					spawnPipeEvery = spawnPipeEvery * 0.98;
				end

				if x1 < (0.0 - pxToScreenX(pipe_xSize + 5)) then
					-- Destroy objects
					scene_manager:RemoveObject("sprite_pipe_down"..tostring(pipeIdx));
					scene_manager:RemoveObject("sprite_pipe_up"..tostring(pipeIdx));
					spawnPipeFirstIdx = spawnPipeFirstIdx + 1;
				end
			end
		end
		
		-- Check collisions with grounds
		if 	checkCollisions2DBox(birbx, birby,						pxToScreenX(birb_xSize), pxToScreenY(birb_ySize),
								 0.0,	 0.0,						1.0,					 pxToScreenY(60)) or
			checkCollisions2DBox(birbx, birby, 						pxToScreenX(birb_xSize), pxToScreenY(birb_ySize),
								  0.0,	pxToScreenY(screenY - 60), 	1.0, 					 pxToScreenY(60))
		then
			gameIsGameOver = true;
			cmepmeta.logger.SimpleLog(string.format("Game over!"))

			local font = asset_manager:GetFont("myfont");
			local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.4, 0.4, -0.01, 32, "GAME OVER", font);
			scene_manager:AddObject("text_gameover", object);
			return 0;
		end

		-- Fall birb
		local birb = scene_manager:FindObject("birb");
		local x, y, z = birb:GetPosition();
		y = y - birbVelocity * event.deltaTime;
		birb:Translate(x, y, z);

		local ground1 = scene_manager:FindObject("ground_top");
		local ground2 = scene_manager:FindObject("ground_bottom");
		local g1_x, g1_y, g1_z = ground1:GetPosition();
		local g2_x, g2_y, g2_z = ground2:GetPosition();

		g1_x = g1_x - (pipeMoveSpeed * 1.05) * event.deltaTime;
		g2_x = g2_x - (pipeMoveSpeed * 1.05) * event.deltaTime;

		if	(g1_x > pxToScreenX(-120)) or
			(g2_x > pxToScreenX(-120))
		then
			ground1:Translate(g1_x, g1_y, g1_z);
			ground2:Translate(g2_x, g2_y, g2_z);
		else
			ground1:Translate(0.0, g1_y, g1_z);
			ground2:Translate(0.0, g2_y, g2_z);
		end
			--cmepmeta.logger.SimpleLog(string.format("Valid pos %f", g1_x))
			--g1_x = g1_x - (pipeMoveSpeed * 1.05) * event.deltaTime;
			--g2_x = g2_x - (pipeMoveSpeed * 1.05) * event.deltaTime;
		--else
		--	cmepmeta.logger.SimpleLog(string.format("Reset on %f", g1_x))
		--	g1_x = 0.0
		--	g2_x = 0.0
		--end

		--ground1:Translate(g1_x, g1_y, g1_z);
		--ground2:Translate(g2_x, g2_y, g2_z);

		birbVelocity = birbVelocity - birbFallSpeed * event.deltaTime;
		spawnPipeSinceLast = spawnPipeSinceLast + event.deltaTime;
	end

	return 0;
end

onInit = function(event)
	event.engine:SetFramerateTarget(event.engine, 0); -- VSYNC enabled if target is 0
	--cmepapi.EngineSetFramerateTarget(event.engine, 0); -- VSYNC enabled

	-- Get managers
	local asset_manager = event.engine:GetAssetManager();
	local scene_manager = event.engine:GetSceneManager();
	--local asset_manager = cmepapi.EngineGetAssetManager(event.engine);
	--local scene_manager = cmepapi.EngineGetSceneManager(event.engine);

	-- Create frametime counter and add it to scene
	local font = asset_manager:GetFont("myfont");
	local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.0, 0.0, -0.01, 22, "test", font);
	scene_manager:AddObject("_debug_info", object);
	
	-- Add score
	local object = cmepapi.ObjectFactoryCreateTextObject(scene_manager, 0.5, 0.0, -0.01, 64, "0", font);
	scene_manager:AddObject("text_score", object);

	-- Set-up camera
	scene_manager:SetCameraTransform(0.0, 0.0, 0.0);
	scene_manager:SetCameraHVRotation(0, 0);

	-- Set-up light
	scene_manager:SetLightTransform(-1, 1, 0);

	return 0;
end

--->  Game events  <---
-----------------------
