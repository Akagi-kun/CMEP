#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
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

		void RebuildVBO(Vulkan::VulkanRenderingEngine* renderer)
		{
			delete this->vbo;
			this->vbo			 = renderer->CreateVulkanVertexBufferFromData(this->mesh);
			this->vbo_vert_count = this->mesh.size();
			this->been_rebuilt	 = true;
		}
	};
} // namespace Engine::Rendering
