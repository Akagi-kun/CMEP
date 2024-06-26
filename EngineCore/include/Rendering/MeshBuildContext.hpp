#pragma once

#include "Vulkan/VulkanStructDefs.hpp"
namespace Engine::Rendering
{
	struct MeshBuildContext
	{
		// Build storage
		std::vector<RenderingVertex> mesh;

		// On build finish
		VulkanBuffer* vbo;
		size_t vbo_vert_count;
	};
} // namespace Engine::Rendering
