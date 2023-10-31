onMouseMoved = function(event)

   local mouseSpeed = 0.1;
   
   local h, v = cmepapi.gsm_GetCameraHVRotation();
   
	h = h + mouseSpeed * event.deltaTime * (event.mouse.x - (1100 / 2));
	v = v - mouseSpeed * event.deltaTime * ((720 / 2) - event.mouse.y);
   
   cmepapi.gsm_SetCameraHVRotation(h, v);
   
   return 0; 
   
end


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
   
   if keycodeSwitchTbl[string.char(event.keycode)] ~= nil then
      keycodeSwitchTbl[string.char(event.keycode)].func();
      
      cmepapi.gsm_SetCameraTransform(transform_x, transform_y, transform_z);
   end
   
   return 0;
end

deltaTimeAvg = 0.0;
deltaTimeCount = 0;

onUpdate = function(event)
   deltaTimeAvg = deltaTimeAvg + event.deltaTime;
   deltaTimeCount = deltaTimeCount + 1;
   
   if deltaTimeCount >= 60 then
      local object = cmepapi.engine_FindObject("_debug_info");
      cmepapi.textRenderer_UpdateText(object.renderer, "FT: "..tostring(deltaTimeAvg / deltaTimeCount * 1000).." ms");
      
      deltaTimeAvg = 0;
      deltaTimeCount = 0;
   end
   
   local object = cmepapi.engine_FindObject("_rot_ctr");
   local h, v = cmepapi.gsm_GetCameraHVRotation();
   cmepapi.textRenderer_UpdateText(object.renderer, "RotX: "..string.format("%.4f", h)..", RotY: "..string.format("%.4f", v));

   
	local obj3d = cmepapi.engine_FindObject("test_3d");
	local xrot, yrot, zrot = cmepapi.object_GetRotation(obj3d);
	yrot = yrot - 15.0 * event.deltaTime;
	cmepapi.object_Rotate(obj3d, xrot, yrot, zrot);
   
	 local obj3d2 = cmepapi.engine_FindObject("test_3d2");
	 local xrot2, yrot2, zrot2 = cmepapi.object_GetRotation(obj3d2);
	 zrot2 = zrot2 - 15.0 * event.deltaTime;
	 cmepapi.object_Rotate(obj3d2, xrot2, yrot2, zrot2);

   --local sinresult = math.sin(deltaTimeAvg);
   --local obj3d = cmepapi.engine_FindObject("test_3d");
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
   cmepapi.engine_AddObject("_debug_info", object);
   
   local font = cmepapi.assetManager_GetFont(asset_manager, "game/fonts/myfont/myfont.fnt");
   local object = cmepapi.objectFactory_CreateTextObject(0.0, 0.03, 18, "test", font);
   cmepapi.engine_AddObject("_rot_ctr", object);
   
   -- Setup test 3D model
   local mesh3d = cmepapi.mesh_Mesh();
   cmepapi.mesh_CreateMeshFromObj(mesh3d, "game/models/cube.obj");
   
   local object3d = cmepapi.objectFactory_CreateGeneric3DObject(3, 0, 0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, mesh3d);
   
   cmepapi.engine_AddObject("test_3d", object3d);
   
   
   local mesh3d2 = cmepapi.mesh_Mesh();
   cmepapi.mesh_CreateMeshFromObj(mesh3d2, "game/models/cube.obj");
   
   local object3d2 = cmepapi.objectFactory_CreateGeneric3DObject(0, 3, 0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, mesh3d2);
   
   cmepapi.engine_AddObject("test_3d2", object3d2);
   
   cmepapi.object_AddChild(object3d, object3d2);
   
   
   -- Set-up camera
   cmepapi.gsm_SetCameraTransform(-5.0, 2.0, 0.0);
   cmepapi.gsm_SetCameraHVRotation(3.15, 3.5);
   
   -- Set-up light
   cmepapi.gsm_SetLightTransform(-1,1,0);
   
   return 0;
end