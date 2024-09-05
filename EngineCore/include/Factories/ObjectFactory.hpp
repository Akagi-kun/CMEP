#pragma once

#include "Rendering/SupplyData.hpp"

#include "EnumStringConvertor.hpp"
#include "Object.hpp"

#include <stdexcept>
#include <string>

namespace Engine::Factories::ObjectFactory
{
	enum class RendererType : uint8_t
	{
		MIN_ENUM = 0x00,

		RENDERER_2D = 2,
		RENDERER_3D = 5,

		MAX_ENUM = 0XFF
	};

	enum class MeshBuilderType : uint8_t
	{
		MIN_ENUM = 0x00,

		TEXT	  = 1,
		SPRITE	  = 2,
		GENERATOR = 3,
		AXIS	  = 4,

		MAX_ENUM = 0XFF
	};

	struct ObjectTemplate
	{
		EnumStringConvertor<RendererType>			  with_renderer;
		EnumStringConvertor<MeshBuilderType>		  with_mesh_builder;
		std::string									  with_shader;
		std::vector<Rendering::RendererSupplyData>	  renderer_supply_list;
		std::vector<Rendering::MeshBuilderSupplyData> meshbuilder_supply_list;
	};

	template <class TRenderer, class TMeshBuilder>
	Object* CreateSceneObject(
		Engine*												 with_engine,
		std::string											 with_pipeline_program,
		const std::vector<Rendering::RendererSupplyData>&	 renderer_supply_data,
		const std::vector<Rendering::MeshBuilderSupplyData>& meshbuilder_supply_data
	)
		requires(
			(std::is_same_v<TRenderer, Rendering::Renderer2D> && TMeshBuilder::supports_2d) ||
			(std::is_same_v<TRenderer, Rendering::Renderer3D> && TMeshBuilder::supports_3d)
		)
	{
		if (with_pipeline_program.empty())
		{
			throw std::invalid_argument("Cannot CreateSceneObject without a pipeline! (was empty)");
		}

		auto* with_builder = new TMeshBuilder(with_engine);

		for (const auto& supply : meshbuilder_supply_data)
		{
			with_builder->SupplyData(supply);
		}

		auto* with_renderer =
			new TRenderer(with_engine, with_builder, with_pipeline_program.c_str());

		for (const auto& supply : renderer_supply_data)
		{
			with_renderer->SupplyData(supply);
		}

		auto* object = new Object(with_engine, with_renderer, with_builder);

		return object;
	}

	using object_factory_t = std::function<
		Object*(Engine*, std::string, const std::vector<Rendering::RendererSupplyData>&, const std::vector<Rendering::MeshBuilderSupplyData>&)>;

	// Returns a Callable capable of creating the desired object
	object_factory_t GetSceneObjectFactory(
		EnumStringConvertor<RendererType>	 with_renderer,
		EnumStringConvertor<MeshBuilderType> with_mesh_builder
	);

	Object* InstantiateObjectTemplate(Engine* with_engine, ObjectTemplate& from_template);

	using supply_data_value_t = std::variant<std::monostate, void*, std::string>;

	Rendering::RendererSupplyData GenerateRendererSupplyData(
		EnumStringConvertor<Rendering::RendererSupplyData::Type> of_type,
		supply_data_value_t										 with_value
	);

	Rendering::MeshBuilderSupplyData GenerateMeshBuilderSupplyData(
		EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> of_type,
		supply_data_value_t											with_value
	);

} // namespace Engine::Factories::ObjectFactory
