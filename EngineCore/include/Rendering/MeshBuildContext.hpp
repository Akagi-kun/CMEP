#pragma once

#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/VBuffer.hpp"

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
