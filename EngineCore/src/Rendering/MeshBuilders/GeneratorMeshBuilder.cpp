#include "Rendering/MeshBuilders/GeneratorMeshBuilder.hpp"

#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/SupplyData.hpp"

#include "Scripting/ILuaScript.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <vector>

namespace Engine::Rendering
{
	void GeneratorMeshBuilder::supplyData(const MeshBuilderSupplyData& data)
	{
		IMeshBuilder::supplyData(data);

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

	void GeneratorMeshBuilder::build()
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
				context.rebuildVBO(instance, mesh);
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
