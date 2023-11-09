-- ON_MOUSEMOVED event, left here for your usage!
onMouseMoved = function(event)
	return 0;
end

-- ON_KEYDOWN event
onKeyDown = function(event)
	return 0;
end

-- ON_KEYUP event
onKeyUp = function(event)
	return 0;
end

-- ON_UPDATE event
onUpdate = function(event)
	cmepapi.gsm_RemoveObject("3DObject2");
	cmepapi.gsm_RemoveObject("3DObject");
	cmepapi.gsm_RemoveObject("birb");
	cmepapi.gsm_RemoveObject("_debug_info");
	
	return 1;
end

onInit = function(event)
	cmepapi.engine_SetFramerateTarget(60); -- VSYNC enabled

	-- Get asset manager
	local asset_manager = cmepapi.engine_GetAssetManager();

	-- Create frametime counter and add it to scene
	local font = cmepapi.assetManager_GetFont(asset_manager, "game/fonts/myfont/myfont.fnt");
	local debugobject = cmepapi.objectFactory_CreateTextObject(0.0, 0.0, 18, "test", font);
	cmepapi.textRenderer_UpdateText(debugobject.renderer, "TESTING");
	cmepapi.gsm_AddObject("_debug_info", debugobject);
	
	-- Add birb
	-- local sprite = cmepapi.assetManager_GetTexture(asset_manager, "game/textures/birb.png");
	local birb = cmepapi.objectFactory_CreateSpriteObject(0.2, (720 / 2) / 720, 48 / 1100, 33 / 720, asset_manager, "game/textures/birb.png");
	cmepapi.gsm_AddObject("birb", birb);

	--local mesh = cmepapi.mesh_Mesh();
	--cmepapi.mesh_CreateMeshFromObj(mesh, "game/models/cube.obj");
	local object3d = cmepapi.objectFactory_CreateGeneric3DObject(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, "game/models/cube.obj")
	cmepapi.gsm_AddObject("3DObject", object3d);
	local object3d2 = cmepapi.objectFactory_CreateGeneric3DObject(0.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, "game/models/cube.obj")
	cmepapi.gsm_AddObject("3DObject2", object3d2);
	cmepapi.object_AddChild(object3d, object3d2);

	local birb = cmepapi.gsm_FindObject("birb");
	local x, y, z = cmepapi.object_GetPosition(birb);
	cmepapi.object_Translate(birb, x, y, z);

	local object3d = cmepapi.gsm_FindObject("3DObject");
	local x, y, z = cmepapi.object_GetRotation(object3d);
	cmepapi.object_Rotate(birb, x, y, z);

	-- Set-up camera
	local cx, cy, cz = cmepapi.gsm_GetCameraTransform();
	cmepapi.gsm_SetCameraTransform(-5.0, 0.0, 0.0);
	local crotx, croty, crotz = cmepapi.gsm_GetCameraHVRotation();
	cmepapi.gsm_SetCameraHVRotation(0, 0);

	-- Set-up light
	local lx, ly, lz = cmepapi.gsm_GetLightTransform();
	cmepapi.gsm_SetLightTransform(-1,1,0);

	return 0;
end