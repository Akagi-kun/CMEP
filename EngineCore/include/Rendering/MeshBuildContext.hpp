#pragma once

#include "Vulkan/VBuffer.hpp"
#include "Vulkan/VulkanStructDefs.hpp"
namespace Engine::Rendering
{
	struct MeshBuildContext
	{
		// Build storage
		std::vector<RenderingVertex> mesh;

		// On build finish
		Vulkan::VBuffer* vbo;
		size_t vbo_vert_count;

		bool been_rebuilt;
	};
} // namespace Engine::Rendering
