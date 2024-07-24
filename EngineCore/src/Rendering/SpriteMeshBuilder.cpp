#include "Rendering/SpriteMeshBuilder.hpp"

#include "Rendering/MeshBuildContext.hpp"

#include <iterator>

namespace Engine::Rendering
{
	void SpriteMeshBuilder::Build()
	{
		if (this->context.vbo == nullptr)
		{
			this->context = MeshBuildContext();

			// Simple quad mesh
			const std::vector<RenderingVertex> generated_mesh = {
				RenderingVertex{glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0, 1.0)},
				RenderingVertex{glm::vec3(1.0, 1.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.0, 1.0)},
				RenderingVertex{glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0, 0.0)},
				RenderingVertex{glm::vec3(1.0, 1.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.0, 1.0)},
				RenderingVertex{glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.0, 0.0)},
				RenderingVertex{glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0, 0.0)},
			};

			// Copy into context
			std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(this->context.mesh));

			this->context.RebuildVBO(this->renderer);
		}
	}
} // namespace Engine::Rendering
