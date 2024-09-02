#include "Rendering/MeshBuilders/GeneratorMeshBuilder.hpp"

#include "Rendering/SupplyData.hpp"

#include "Scripting/ILuaScript.hpp"

#include <iterator>

namespace Engine::Rendering
{
	void GeneratorMeshBuilder::SupplyData(const MeshBuilderSupplyData& data)
	{
		switch (data.type)
		{
			case Rendering::MeshBuilderSupplyData::Type::GENERATOR:
			{
				script_data = std::get<GeneratorData>(data.payload);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	void GeneratorMeshBuilder::Build()
	{
		if (context.vbo == nullptr)
		{
			std::vector<RenderingVertex> generated_mesh;

			Scripting::ScriptFunctionRef supplier = script_data.supplier;

			std::array<void*, 3> generator_data = {&generated_mesh, &supplier, &(world_pos)};

			script_data.generator(&generator_data);

			if (!generated_mesh.empty())
			{
				// Create context
				std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(mesh));

				context = MeshBuildContext();
				context.RebuildVBO(instance, mesh);
			}
			else
			{
				// Renderers shall skip the render if there are no vertices
				context.vbo_vert_count = 0;
			}
			needs_rebuild = false;
		}
	}
} // namespace Engine::Rendering
