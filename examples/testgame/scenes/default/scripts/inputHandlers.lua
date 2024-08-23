
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

local forwardVector = function(yaw, pitch)
	local front_x = math.cos(yaw) * math.cos(pitch);
	local front_y = math.sin(pitch);
	local front_z = math.sin(yaw) * math.cos(pitch);

	return front_x, front_y, front_z
end

local keystates = {false, false, false, false, false}

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

	local keycodeSwitchTbl = {
	   [string.byte('W')] = function()
			keystates[1] = true
	   end,
	   [string.byte('S')] = function()
			keystates[2] = true
	   end,
	   [string.byte('A')] = function()
			keystates[3] = true
	   end,
	   [string.byte('D')] = function()
			keystates[4] = true
	   end,
	   [340] = function() -- shift key is value 340
			keystates[5] = true
	   end
	};
	
	if keycodeSwitchTbl[event.keycode] ~= nil then
	   keycodeSwitchTbl[event.keycode]();
	end

	return 0
end

-- ON_KEYUP event
-- 
-- this event is called exactly once for every release of a key
-- 
onKeyUp = function(event)
	local keycodeSwitchTbl = {
	   [string.byte('W')] = function()
			keystates[1] = false
	   end,
	   [string.byte('S')] = function()
			keystates[2] = false
	   end,
	   [string.byte('A')] = function()
			keystates[3] = false
	   end,
	   [string.byte('D')] = function()
			keystates[4] = false
	   end,
	   [340] = function() -- shift key is value 340
			keystates[5] = false
	   end
	};

	if keycodeSwitchTbl[event.keycode] ~= nil then
		keycodeSwitchTbl[event.keycode]();
	 end

	return 0
end

onMovementTick = function(event)
	local moveSpeed = 15.0 * event.deltaTime;
	  
	local scene_manager = event.engine:GetSceneManager()
	local camera_h, camera_v = scene_manager:GetCameraHVRotation();

	local pitch = math.rad(camera_v)
	local yaw = math.rad(camera_h) 

	local front_x, front_y, front_z = forwardVector(yaw, pitch)
	local right_x, right_y, right_z = vectorCross(front_x, front_y, front_z, 0, 1, 0)

	-- Ignore pitch for side-to-side movement
	local front_x2, front_y2, front_z2 = forwardVector(yaw, math.rad(180))
	local right_x2, right_y2, right_z2 = vectorCross(front_x2, front_y2, front_z2, 0, 1, 0)

	local up_x, up_y, up_z = vectorCross(right_x, right_y, right_z, front_x, front_y, front_z)

	local transform_x, transform_y, transform_z = scene_manager:GetCameraTransform();

	local keycodeSwitchTbl = {
	  	function() -- forward
			transform_x = transform_x + front_x * moveSpeed;
			transform_y = transform_y + front_y * moveSpeed;
			transform_z = transform_z + front_z * moveSpeed;
	   end,
		function() -- backward
			transform_x = transform_x - front_x * moveSpeed;
			transform_y = transform_y - front_y * moveSpeed;
			transform_z = transform_z - front_z * moveSpeed;
	   end,
	   function() -- left
			transform_x = transform_x - right_x2 * moveSpeed;
			transform_y = transform_y - right_y2 * moveSpeed;
			transform_z = transform_z - right_z2 * moveSpeed;
	   end,
	   function() -- right
			transform_x = transform_x + right_x2 * moveSpeed;
			transform_y = transform_y + right_y2 * moveSpeed;
			transform_z = transform_z + right_z2 * moveSpeed;
	   end,
	   function() -- down
			transform_x = transform_x - up_x * moveSpeed;
			transform_y = transform_y - up_y * moveSpeed;
			transform_z = transform_z - up_z * moveSpeed;
	   end
	};
	
	for i = 1, 5 do
		if keystates[i] == true then
			keycodeSwitchTbl[i]();

			scene_manager:SetCameraTransform(transform_x, transform_y, transform_z);
		end
	end
	return 0
end
