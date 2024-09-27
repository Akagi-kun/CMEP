#pragma once

#include "Rendering/Vulkan/backend.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include <cstddef>
#include <vector>

namespace Engine::Rendering
{
	struct MeshBuildContext
	{
		Vulkan::Buffer* vbo;
		size_t			vbo_vert_count;

		void
		rebuildVBO(Vulkan::Instance* with_instance, const std::vector<RenderingVertex>& mesh)
		{
			delete vbo;

			auto command_buffer = with_instance->getCommandPool()->constructCommandBuffer();

			vbo = new Vulkan::VertexBuffer(
				with_instance->getLogicalDevice(),
				with_instance->getGraphicMemoryAllocator(),
				command_buffer,
				mesh
			);
			vbo_vert_count = mesh.size();
		}
	};
} // namespace Engine::Rendering
