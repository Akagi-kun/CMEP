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
			this->generator_script = std::static_pointer_cast<Scripting::ILuaScript>(data.payload_ptr);
		}
	}

	void GeneratorMeshBuilder::Build()
	{
		if (this->context.vbo == nullptr)
		{
			std::vector<RenderingVertex> generated_mesh;

			this->generator_script->CallFunction("GENERATOR_FUNCTION", &generated_mesh);

			// Create context
			std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(this->mesh));

			this->context = MeshBuildContext();
			this->context.RebuildVBO(this->renderer, this->mesh);
			this->needs_rebuild = false;
		}
	}
} // namespace Engine::Rendering
