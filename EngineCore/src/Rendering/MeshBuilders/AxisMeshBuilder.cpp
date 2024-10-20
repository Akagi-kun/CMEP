#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"

#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/Vulkan/common.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

namespace Engine::Rendering
{
	void AxisMeshBuilder::build()
	{
		if (context.vbo == nullptr)
		{
			// Simple quad mesh
			const std::vector<RenderingVertex> generated_mesh = {
				RenderingVertex{{0.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}},
				RenderingVertex{{1.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}},
				RenderingVertex{{0.0, 0.0, 0.0}, {0.0f, 0.0f, 1.0f}},
				RenderingVertex{{0.0, 1.0, 0.0}, {0.0f, 0.0f, 1.0f}},
				RenderingVertex{{0.0, 0.0, 0.0}, {1.0f, 0.0f, 0.0f}},
				RenderingVertex{{0.0, 0.0, 1.0}, {1.0f, 0.0f, 0.0f}}
			};

			// Create context
			std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(mesh));

			context = MeshBuildContext();
			context.rebuildVBO(instance, mesh);
			needs_rebuild = false;
		}
	}
} // namespace Engine::Rendering
