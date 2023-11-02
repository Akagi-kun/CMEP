onMouseMoved = function(event)

   local mouseSpeed = 0.05;

   local h, v = cmepapi.gsm_GetCameraHVRotation();

	h = h + mouseSpeed * event.deltaTime * event.mouse.x;
	v = v + mouseSpeed * event.deltaTime * event.mouse.y;

   cmepapi.gsm_SetCameraHVRotation(h, v);

   return 0;

end

birbVelocity = 0.1;
birbIsVelociting = true;

onKeyDown = function(event)

   local moveSpeed = 2.5;

   local camera_h, camera_v = cmepapi.gsm_GetCameraHVRotation();

   local direction_x = math.cos(camera_v) * math.cos(camera_h);
   local direction_y = math.sin(camera_v);
   local direction_z = math.cos(camera_v) * math.sin(camera_h);

   --{x = v1.y*v2.z - v2.y*v1.z , y = v2.x*v1.z-v1.x*v2.z , z = v1.x*v2.y-v2.x*v1.y}
   local right_x = direction_x * 0.0 - 1.0 * direction_z;--math.sin(camera_h - 3.14 / 2.0);
   local right_y = 0.0 * direction_z - direction_x * 0.0;--0;
   local right_z = direction_x * 1.0 - 0.0 * direction_y;--math.cos(camera_h - 3.14 / 2.0)

   local transform_x, transform_y, transform_z = cmepapi.gsm_GetCameraTransform();

   local keycodeSwitchTbl = {
      ['W'] = {func = function()
         transform_x = transform_x + direction_x * moveSpeed * event.deltaTime;
         transform_y = transform_y + direction_y * moveSpeed * event.deltaTime;
         transform_z = transform_z + direction_z * moveSpeed * event.deltaTime;
      end},
      ['S'] = {func = function()
         transform_x = transform_x - direction_x * moveSpeed * event.deltaTime;
         transform_y = transform_y - direction_y * moveSpeed * event.deltaTime;
         transform_z = transform_z - direction_z * moveSpeed * event.deltaTime;
      end},
      ['A'] = {func = function()
         transform_x = transform_x - right_x * moveSpeed * event.deltaTime;
         transform_y = transform_y - right_y * moveSpeed * event.deltaTime;
         transform_z = transform_z - right_z * moveSpeed * event.deltaTime;
      end},
      ['D'] = {func = function()
         transform_x = transform_x + right_x * moveSpeed * event.deltaTime;
         transform_y = transform_y + right_y * moveSpeed * event.deltaTime;
         transform_z = transform_z + right_z * moveSpeed * event.deltaTime;
      end}
   };

   if string.char(event.keycode) == ' ' then
      if birbIsVelociting == false then
         birbVelocity = birbVelocity + 0.5;
         birbIsVelociting = true;
      end
   end

   if keycodeSwitchTbl[string.char(event.keycode)] ~= nil then
      keycodeSwitchTbl[string.char(event.keycode)].func();

      cmepapi.gsm_SetCameraTransform(transform_x, transform_y, transform_z);
   end

   return 0;
end


onKeyUp = function(event)
   if string.char(event.keycode) == ' ' then
      birbIsVelociting = false;
   end
end

deltaTimeAvg = 0.0;
deltaTimeCount = 0;

spawnPipeSinceLast = 0.0;
spawnPipeEvery = 8.0;
spawnPipeLastIdx = 0;
spawnPipeFirstIdx = 1;
spawnPipeCount = 0;

gameIsGameOver = false;

onUpdate = function(event)
   deltaTimeAvg = deltaTimeAvg + event.deltaTime;
   deltaTimeCount = deltaTimeCount + 1;

   if deltaTimeCount >= 30 then
      local object = cmepapi.gsm_FindObject("_debug_info");
      cmepapi.textRenderer_UpdateText(object.renderer, "FT: "..tostring(deltaTimeAvg / deltaTimeCount * 1000).." ms");

      deltaTimeAvg = 0;
      deltaTimeCount = 0;
   end

   --local object = cmepapi.gsm_FindObject("_rot_ctr");
   --local h, v = cmepapi.gsm_GetCameraHVRotation();
   --cmepapi.textRenderer_UpdateText(object.renderer, "RotX: "..string.format("%.4f", h)..", RotY: "..string.format("%.4f", v));

   local offset = -100 + math.random(-100, 100);

   local asset_manager = cmepapi.engine_GetAssetManager();
   if gameIsGameOver == false then
      if spawnPipeSinceLast > spawnPipeEvery then
         -- Spawn new pipes
         local sprite1 = cmepapi.assetManager_GetTexture(asset_manager, "game/textures/pipe_down.png");
         local object1 = cmepapi.objectFactory_CreateSpriteObject(1.0, offset / 720, 80 / 1100, 400 / 720, sprite1);
         cmepapi.gsm_AddObject("sprite_pipe_down"..tostring(spawnPipeLastIdx + 1), object1);

         local sprite2 = cmepapi.assetManager_GetTexture(asset_manager, "game/textures/pipe_up.png");
         local object2 = cmepapi.objectFactory_CreateSpriteObject(1.0, (400 + 200 + offset) / 720, 80 / 1100, 400 / 720, sprite2);
         cmepapi.gsm_AddObject("sprite_pipe_up"..tostring(spawnPipeLastIdx + 1), object2);

         spawnPipeLastIdx = spawnPipeLastIdx + 1;
         spawnPipeCount = spawnPipeCount + 1;
         spawnPipeSinceLast = 0.0;
      end

      local birb = cmepapi.gsm_FindObject("birb");
      local birbx, birby, birbz = cmepapi.object_GetPosition(birb);

      if spawnPipeCount >= 1 then
         for pipeIdx = spawnPipeFirstIdx, spawnPipeLastIdx, 1 do
            -- Move pipes
            local pipe1 = cmepapi.gsm_FindObject("sprite_pipe_down"..tostring(pipeIdx));
            local pipe2 = cmepapi.gsm_FindObject("sprite_pipe_up"..tostring(pipeIdx));
            local x1, y1, z1 = cmepapi.object_GetPosition(pipe1);
            local x2, y2, z2 = cmepapi.object_GetPosition(pipe2);
            x1 = x1 - 0.05 * event.deltaTime;
            x2 = x2 - 0.05 * event.deltaTime;
            cmepapi.object_Translate(pipe1, x1, y1, z1);
            cmepapi.object_Translate(pipe2, x2, y2, z2);

            -- Check collisions
            if ((birbx > x1) and birbx < (x1 + (80 / 1100))) and ((birby > y1) and (birby < (y1 + (400 / 720)))) or ((birbx > x2) and birbx < (x2 + (80 / 1100))) and ((birby > y2) and (birby < (y2 + (400 / 720)))) then
               gameIsGameOver = true;
               local font = cmepapi.assetManager_GetFont(asset_manager, "game/fonts/myfont/myfont.fnt");
               local object = cmepapi.objectFactory_CreateTextObject(0.4, 0.4, 32, "GAME OVER", font);
               cmepapi.gsm_AddObject("text_gameover", object);
            end

            if x1 < (0.0 - 79 / 1100) then
               -- Destroy objects
               cmepapi.gsm_RemoveObject("sprite_pipe_down"..tostring(pipeIdx));
               cmepapi.gsm_RemoveObject("sprite_pipe_up"..tostring(pipeIdx));
               spawnPipeFirstIdx = spawnPipeFirstIdx + 1;
            end
         end
      end

      local birb = cmepapi.gsm_FindObject("birb");
      local x, y, z = cmepapi.object_GetPosition(birb);
      y = y - birbVelocity * event.deltaTime;
      cmepapi.object_Translate(birb, x, y, z);

      birbVelocity = birbVelocity - 0.5 * event.deltaTime;

      spawnPipeSinceLast = spawnPipeSinceLast + event.deltaTime;
   end

   --local sinresult = math.sin(deltaTimeAvg);
   --local obj3d = cmepapi.gsm_FindObject("test_3d");
	--cmepapi.object_Translate(obj3d, sinresult, 0, 0);

   return 0;
end

onInit = function(event)
   cmepapi.engine_SetFramerateTarget(0); -- VSYNC enabled

   -- Get asset manager
	local asset_manager = cmepapi.engine_GetAssetManager();

   -- Create frametime counter and add it to scene
   local font = cmepapi.assetManager_GetFont(asset_manager, "game/fonts/myfont/myfont.fnt");
   local object = cmepapi.objectFactory_CreateTextObject(0.0, 0.0, 18, "test", font);
   cmepapi.gsm_AddObject("_debug_info", object);

   local sprite = cmepapi.assetManager_GetTexture(asset_manager, "game/textures/birb.png");
   local object = cmepapi.objectFactory_CreateSpriteObject(0.2, (720 / 2) / 720, 48 / 1100, 33 / 720, sprite);
   cmepapi.gsm_AddObject("birb", object);

   --local font = cmepapi.assetManager_GetFont(asset_manager, "game/fonts/myfont/myfont.fnt");
   --local object = cmepapi.objectFactory_CreateTextObject(0.0, 0.03, 18, "test", font);
   --cmepapi.gsm_AddObject("_rot_ctr", object);

   --local sprite = cmepapi.assetManager_GetTexture(asset_manager, "game/textures/pipe_down.png");
   --local object = cmepapi.objectFactory_CreateSpriteObject(0.0, 0.0, 128 / 1100, 512 / 720, sprite);
   --cmepapi.gsm_AddObject("_test_sprite", object);

   -- Set-up camera
   cmepapi.gsm_SetCameraTransform(-5.0, 0.0, 0.0);
   cmepapi.gsm_SetCameraHVRotation(0, 0);

   -- Set-up light
   cmepapi.gsm_SetLightTransform(-1,1,0);

   return 0;
end