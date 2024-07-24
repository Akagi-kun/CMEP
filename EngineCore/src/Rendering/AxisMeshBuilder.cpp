#include "Rendering/AxisMeshBuilder.hpp"

#include <iterator>

namespace Engine::Rendering
{
	void AxisMeshBuilder::Build()
	{
		if (this->context.vbo == nullptr)
		{
			this->context = MeshBuildContext();

			// Simple quad mesh
			const std::vector<RenderingVertex> generated_mesh = {
				RenderingVertex{glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0f, 1.0f, 0.0f}},
				RenderingVertex{glm::vec3{1.0, 0.0, 0.0}, glm::vec3{0.0f, 1.0f, 0.0f}},
				RenderingVertex{glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0f, 0.0f, 1.0f}},
				RenderingVertex{glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0f, 0.0f, 1.0f}},
				RenderingVertex{glm::vec3{0.0, 0.0, 0.0}, glm::vec3{1.0f, 0.0f, 0.0f}},
				RenderingVertex{glm::vec3{0.0, 0.0, 1.0}, glm::vec3{1.0f, 0.0f, 0.0f}}
			};

			// Copy into context
			std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(this->context.mesh));

			this->context.RebuildVBO(this->renderer);
		}
	}
} // namespace Engine::Rendering
