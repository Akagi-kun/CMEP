local block_types_impl = {
	AIR = 0,
	STONE = 1,
	GRASS = 2,
	DIRT = 3,
	WOOD = 4,
	LEAVES = 5,
	SAND = 6,
	FLOWER = 7,
	MISSING = 8
}

local leaves = block_types_impl.LEAVES
local wood = block_types_impl.WOOD

local tree_def_impl = {
	{
		0, 0, 0, 	0, 0,
		0, 0, 0, 	0, 0,
		0, 0, wood, 0, 0,
		0, 0, 0, 	0, 0,
		0, 0, 0, 	0, 0,
	},
	{
		0, 0, 0, 	0, 0,
		0, 0, 0, 	0, 0,
		0, 0, wood, 0, 0,
		0, 0, 0, 	0, 0,
		0, 0, 0, 	0, 0,
	},
	{
		0, 0, 0, 	0, 0,
		0, 0, 0, 	0, 0,
		0, 0, wood, 0, 0,
		0, 0, 0, 	0, 0,
		0, 0, 0, 	0, 0,
	},
	{
		0, 5, 5, 	5, 0,
		5, 5, 5, 	5, 5,
		5, 5, wood, 5, 5,
		5, 5, 5, 	5, 5,
		0, 5, 5, 	5, 0,
	},
	{
		0, 		leaves, leaves, leaves, 0,
		leaves, leaves, leaves, leaves, leaves,
		leaves, leaves, wood, 	leaves, leaves,
		leaves, leaves, leaves, leaves, leaves,
		0, 		leaves, leaves, leaves, 0,
	},
	{
		0, 0, 		0, 		0,		0,
		0, leaves,	leaves, leaves, 0,
		0, leaves,	leaves, leaves, 0,
		0, leaves,	leaves, leaves, 0,
		0, 0, 		0, 		0, 		0,
	},
	{
		0, 0, 		0, 		0, 		0,
		0, 0, 		leaves, 0, 		0,
		0, leaves,	leaves, leaves, 0,
		0, 0, 		leaves, 0, 		0,
		0, 0, 		0, 		0, 		0,
	}
}

return {
	block_types = block_types_impl,

	blocks_transparent = { block_types_impl.AIR, 128 }, -- 128 chunk boundary
	blocks_crossfaced = { block_types_impl.FLOWER },

	tree_def = tree_def_impl,

	block_type_count = 11,
	block_textures = 2,

	texture_pixels = { x = 10, y = 10 }
}