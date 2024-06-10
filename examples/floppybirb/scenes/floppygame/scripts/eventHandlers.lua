-- ON_MOUSEMOVED event, left here for your usage!
onMouseMoved = function(event)
	return 0;
end

birbVelocity = 0.1;
birbIsVelociting = true;

-- ON_KEYDOWN event
onKeyDown = function(event)
	-- check for esc
	if event.keycode == 256 then
		cmepapi.engine_Stop(event.engine);
		return 0;
	end

	-- check for space press
	if string.char(event.keycode) == ' ' then
		if birbIsVelociting == false then
			birbVelocity = 0.3;
			birbIsVelociting = true;
		end
	end

	return 0;
end

-- ON_KEYUP event
onKeyUp = function(event)
	if string.char(event.keycode) == ' ' then
		birbIsVelociting = false;
	end
end

---------------------
----  Game data  ----

local deltaTimeAvg = 0.0;
local deltaTimeCount = 0;

local spawnPipeEvery = 5.0;
local spawnPipeSinceLast = spawnPipeEvery - 0.1;
local spawnPipeLastIdx = 0;
local spawnPipeFirstIdx = 1;
local spawnPipeCount = 0;

-- Pipes original size is 110x338
local pipe_xSize <const> = 110;
local pipe_ySize <const> = 450;
local pipe_spacing_start <const> = 200;
local pipe_spacing = pipe_spacing_start;

local birb_xSize <const> = 72;
local birb_ySize <const> = 44;

local gameIsGameOver = false;
local gameLastScoredPipeIdx = 0;
local gameScore = 0;
local pipeMoveSpeed = 0.1;

----  Game data  ----
---------------------

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

-- ON_UPDATE event
onUpdate = function(event)
	deltaTimeAvg = deltaTimeAvg + event.deltaTime;
	deltaTimeCount = deltaTimeCount + 1;

	local asset_manager = cmepapi.engine_GetAssetManager(event.engine);
	local scene_manager = cmepapi.engine_GetSceneManager(event.engine);

	-- Update frametime counter, recommend to leave this here for debugging purposes
	if deltaTimeCount >= 30 then
		--cmepmeta.logger.SimpleLog(string.format("Hello from Lua! Last FT is: %f ms!", deltaTimeAvg / deltaTimeCount * 1000))
		--cmepmeta.logger.SimpleLog(string.format("Test (%f, %f) (%f, %f)", pxToScreenX(80), pxToScreenY(400), 80/1100, 400/720))
		--local object = cmepapi.sm_FindObject(scene_manager, "_debug_info");
		local object = scene_manager:FindObject("_debug_info");
		cmepapi.textRenderer_UpdateText(object.renderer, "FT: "..tostring(deltaTimeAvg / deltaTimeCount * 1000).." ms");
		
		deltaTimeAvg = 0;
		deltaTimeCount = 0;
	end

	-- We can use the math library in here!
	local offset = -200 + math.random(-15, 15) * 10;

	if gameIsGameOver == false then
		if spawnPipeSinceLast > spawnPipeEvery then
			-- Spawn new pipes

			--local object1 = cmepapi.objectFactory_CreateSpriteObject(scene_manager, 1.0, offset / 720, 80 / 1100, 400 / 720, asset_manager, "textures/pipe_down.png");
			local object1 = scene_manager:AddTemplatedObject("sprite_pipe_down"..tostring(spawnPipeLastIdx + 1), "pipe_down");
			object1:Translate(1.0, pxToScreenY(offset), -0.15);
			object1:Scale(pxToScreenX(pipe_xSize), pxToScreenY(pipe_ySize), 1.0);

			--local object2 = cmepapi.objectFactory_CreateSpriteObject(scene_manager, 1.0, (400 + 200 + offset) / 720, 80 / 1100, 400 / 720, asset_manager, "textures/pipe_up.png");
			local object2 = scene_manager:AddTemplatedObject("sprite_pipe_up"..tostring(spawnPipeLastIdx + 1), "pipe_up");
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
					local object = cmepapi.objectFactory_CreateTextObject(scene_manager, 0.4, 0.4, -0.01, 32, "GAME OVER", font);
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
					cmepapi.textRenderer_UpdateText(score_object.renderer, tostring(gameScore));

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
			local object = cmepapi.objectFactory_CreateTextObject(scene_manager, 0.4, 0.4, -0.01, 32, "GAME OVER", font);
			scene_manager:AddObject("text_gameover", object);
			return 0;
		end

		local birb = scene_manager:FindObject("birb");
		local x, y, z = birb:GetPosition();
		y = y - birbVelocity * event.deltaTime;
		birb:Translate(x, y, z);

		local ground1 = scene_manager:FindObject("ground_top");
		local ground2 = scene_manager:FindObject("ground_bottom");
		local g1_x, g1_y, g1_z = ground1:GetPosition();
		local g2_x, g2_y, g2_z = ground2:GetPosition();

		if	(g1_x > pxToScreenX(-60) + 0.001) or
			(g2_x > pxToScreenX(-60) + 0.001)
		then
			g1_x = g1_x - (pipeMoveSpeed * 1.05) * event.deltaTime;
			g2_x = g2_x - (pipeMoveSpeed * 1.05) * event.deltaTime;
		else
			g1_x = 0.0
			g2_x = 0.0
		end

		ground1:Translate(g1_x, g1_y, g1_z);
		ground2:Translate(g2_x, g2_y, g2_z);

		birbVelocity = birbVelocity - 0.65 * event.deltaTime;
		spawnPipeSinceLast = spawnPipeSinceLast + event.deltaTime;
	end

	--local camrotx, camroty = scene_manager:GetCameraHVRotation();
	--scene_manager:SetCameraHVRotation(camrotx, camroty + 60.0 * event.deltaTime);

	return 0;
end

onInit = function(event)
	cmepapi.engine_SetFramerateTarget(event.engine, 0); -- VSYNC enabled

	-- Get managers
	local asset_manager = cmepapi.engine_GetAssetManager(event.engine);
	local scene_manager = cmepapi.engine_GetSceneManager(event.engine);

	-- Create frametime counter and add it to scene
	local font = asset_manager:GetFont("myfont");
	local object = cmepapi.objectFactory_CreateTextObject(scene_manager, 0.0, 0.0, -0.01, 22, "test", font);
	scene_manager:AddObject("_debug_info", object);
	
	-- Add score
	local object = cmepapi.objectFactory_CreateTextObject(scene_manager, 0.5, 0.0, -0.01, 64, "0", font);
	scene_manager:AddObject("text_score", object);

	-- Add birb
	--local birb = scene_manager:AddTemplatedObject("birb", "birb");
	--birb:Translate(0.2, pxToScreenY(screenY / 2), -0.1);
	--birb:Scale(pxToScreenX(birb_xSize), pxToScreenY(birb_ySize), 1.0);

	-- Set-up camera
	scene_manager:SetCameraTransform(0.0, 0.0, 0.0);
	scene_manager:SetCameraHVRotation(0, 0);

	-- Set-up light
	scene_manager:SetLightTransform(-1, 1, 0);

	return 0;
end