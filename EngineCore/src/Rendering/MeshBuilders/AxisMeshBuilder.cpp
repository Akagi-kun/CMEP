#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"

#include <iterator>

namespace Engine::Rendering
{
	/* void AxisMeshBuilder::SupplyData(const RendererSupplyData& data)
	{
		IMeshBuilder::SupplyData(data);
		(void)(data);
	} */

	void AxisMeshBuilder::Build()
	{
		if (context.vbo == nullptr)
		{
			// Simple quad mesh
			const std::vector<RenderingVertex> generated_mesh = {
				RenderingVertex{glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0f, 1.0f, 0.0f}},
				RenderingVertex{glm::vec3{1.0, 0.0, 0.0}, glm::vec3{0.0f, 1.0f, 0.0f}},
				RenderingVertex{glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0f, 0.0f, 1.0f}},
				RenderingVertex{glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0f, 0.0f, 1.0f}},
				RenderingVertex{glm::vec3{0.0, 0.0, 0.0}, glm::vec3{1.0f, 0.0f, 0.0f}},
				RenderingVertex{glm::vec3{0.0, 0.0, 1.0}, glm::vec3{1.0f, 0.0f, 0.0f}}
			};

			// Create context
			std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(mesh));

			context = MeshBuildContext();
			context.RebuildVBO(instance, mesh);
			needs_rebuild = false;
		}
	}
} // namespace Engine::Rendering