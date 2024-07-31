local faces = {
	XPOS = 1,
	XNEG = 2,
	YPOS = 3,
	YNEG = 4,
	ZPOS = 5,
	ZNEG = 6
}

local face_defs = {
	{ -- facing positive X
		{1.0, 1.0, 0.0},
		{1.0, 1.0, 1.0},
		{1.0, 0.0, 0.0},
		{1.0, 0.0, 0.0},
		{1.0, 1.0, 1.0},
		{1.0, 0.0, 1.0}
	},
	{ -- facing negative X
		{0.0, 0.0, 0.0},
		{0.0, 1.0, 1.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 1.0, 1.0},
		{0.0, 0.0, 0.0}
	},

	{ -- facing positive Y
		{0.0, 1.0, 1.0},
		{1.0, 1.0, 1.0},
		{0.0, 1.0, 0.0},
		{0.0, 1.0, 0.0},
		{1.0, 1.0, 1.0},
		{1.0, 1.0, 0.0}
	},
	{ -- facing negative Y
		{0.0, 0.0, 0.0},
		{1.0, 0.0, 1.0},
		{0.0, 0.0, 1.0},
		{1.0, 0.0, 0.0},
		{1.0, 0.0, 1.0},
		{0.0, 0.0, 0.0}
	},

	{ -- facing positive Z
		{0.0, 0.0, 1.0},
		{1.0, 1.0, 1.0},
		{0.0, 1.0, 1.0},
		{0.0, 0.0, 1.0},
		{1.0, 0.0, 1.0},
		{1.0, 1.0, 1.0}
	},
	{ -- facing negative Z
		{0.0, 1.0, 0.0},
		{1.0, 1.0, 0.0},
		{0.0, 0.0, 0.0},
		{1.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, 0.0, 0.0}
	}
}

-- Note that idx is 0 here, add +1 to index tables
GENERATOR_FUNCTION = function(idx)
	if idx >= 36 then
		return true, nil
	end

	local face = math.floor(idx / 6) + 1
	local vertice = (idx % 6) + 1

	--print(face, vertice)

	return false, face_defs[face][vertice]
end