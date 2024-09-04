#include "Rendering/MeshBuilders/SpriteMeshBuilder.hpp"

#include <iterator>

namespace Engine::Rendering
{
	void SpriteMeshBuilder::Build()
	{
		if (context.vbo == nullptr)
		{
			// Simple quad mesh
			const std::vector<RenderingVertex> generated_mesh = {
				RenderingVertex{
					glm::vec3(0.0, 1.0, 0.0),
					glm::vec3(1.f, 0.f, 0.f),
					glm::vec2(0.0, 1.0)
				},
				RenderingVertex{
					glm::vec3(1.0, 1.0, 0.0),
					glm::vec3(1.f, 0.f, 0.f),
					glm::vec2(1.0, 1.0)
				},
				RenderingVertex{
					glm::vec3(0.0, 0.0, 0.0),
					glm::vec3(1.f, 0.f, 0.f),
					glm::vec2(0.0, 0.0)
				},
				RenderingVertex{
					glm::vec3(1.0, 1.0, 0.0),
					glm::vec3(1.f, 0.f, 0.f),
					glm::vec2(1.0, 1.0)
				},
				RenderingVertex{
					glm::vec3(1.0, 0.0, 0.0),
					glm::vec3(1.f, 0.f, 0.f),
					glm::vec2(1.0, 0.0)
				},
				RenderingVertex{
					glm::vec3(0.0, 0.0, 0.0),
					glm::vec3(1.f, 0.f, 0.f),
					glm::vec2(0.0, 0.0)
				},
			};

			// Create context
			std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(mesh));

			context = MeshBuildContext();
			context.RebuildVBO(instance, mesh);
			needs_rebuild = false;
		}
	}
} // namespace Engine::Rendering
