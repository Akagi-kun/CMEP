#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/Buffer.hpp"

namespace Engine::Rendering
{
	struct MeshBuildContext
	{
		// On build finish
		Vulkan::Buffer* vbo;
		size_t vbo_vert_count;

		void RebuildVBO(Vulkan::VulkanRenderingEngine* renderer, const std::vector<RenderingVertex>& mesh)
		{
			delete this->vbo;
			this->vbo			 = renderer->CreateVulkanVertexBufferFromData(mesh);
			this->vbo_vert_count = mesh.size();
		}
	};
} // namespace Engine::Rendering
