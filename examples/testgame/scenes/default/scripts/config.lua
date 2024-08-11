return {
    -- Number of chunks to render in every direction
    render_distance = 12,

    -- Noise multiplier
    noise_intensity = 8,
    noise_layer = 0.0, -- Y slice of noise to pick from

    -- Offset of noise from 0
    floor_level = 16,

    tree_generation_chance = 100, -- 360

    -- Chunk size
    chunk_size_x = 16,
    chunk_size_z = 16,
    chunk_size_y = 64,

    block_type_count = 6,
    block_textures = 2
}