#pragma once

#include "Rendering/Vulkan/exports.hpp"

namespace Engine::Rendering
{
	struct MeshBuildContext
	{
		Vulkan::Buffer* vbo;
		size_t vbo_vert_count;

		void RebuildVBO(Vulkan::Instance* with_instance, const std::vector<RenderingVertex>& mesh)
		{
			delete vbo;
			vbo			   = new Vulkan::VertexBuffer(with_instance, mesh);
			vbo_vert_count = mesh.size();
		}
	};
} // namespace Engine::Rendering
