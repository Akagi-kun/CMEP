#include "Rendering/GeneratorMeshBuilder.hpp"

#include "Rendering/SupplyData.hpp"

#include <iterator>
#include <memory>

namespace Engine::Rendering
{
	void GeneratorMeshBuilder::SupplyData(const RendererSupplyData& data)
	{
		if (data.type == RendererSupplyDataType::SCRIPT)
		{
			const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
			assert(!payload_ref.expired() && "Cannot lock expired payload!");

			this->generator_script = std::static_pointer_cast<Scripting::ILuaScript>(payload_ref.lock());
		}
	}

	void GeneratorMeshBuilder::Build()
	{
		if (this->context.vbo == nullptr)
		{
			std::vector<RenderingVertex> generated_mesh;

			std::array<void*, 2> generator_data = {&generated_mesh, &(this->world_pos)};

			this->generator_script->CallFunction("GENERATOR_FUNCTION", &generator_data);

			if (!generated_mesh.empty())
			{
				// Create context
				std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(this->mesh));

				this->context = MeshBuildContext();
				this->context.RebuildVBO(this->renderer, this->mesh);
				this->needs_rebuild = false;
			}
			else
			{
				// Renderers shall skip the render if there are no vertices
				this->context.vbo_vert_count = 0;
			}
		}
	}
} // namespace Engine::Rendering
