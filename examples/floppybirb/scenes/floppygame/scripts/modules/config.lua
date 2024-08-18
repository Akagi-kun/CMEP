return {
	-- Related to spawning pipes
	spawn_pipe_every_start = 3.2, -- Configurable spawn rate

	-- Pipe sprite size in pixels (note: pipe original size is 110x338)
	pipe_x_size = 110,
	pipe_y_size = 450,

	-- Pipe behavior
	pipe_spacing_start = 200, -- Configurable pipe spacing (between top and bottom)
	pipe_move_speed = 0.155, -- 0.135 Configurable pipe move speed (/speed of flight)

	-- Birb sprite size in pixels
	birb_x_size = 72,
	birb_y_size = 44,

	-- Birb behavior
	birb_jump_velocity = 0.46, -- Configurable velocity set on jump
	birb_gravity = 0.89, -- Configurable gravity
}