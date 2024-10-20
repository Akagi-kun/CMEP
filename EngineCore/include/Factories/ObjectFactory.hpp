#pragma once

#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/SupplyData.hpp"

#include "EnumStringConvertor.hpp"
#include "Exception.hpp"
#include "InternalEngineObject.hpp"
#include "SceneObject.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

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

	/**
	 * A generic template for creating scene objects
	 *
	 * @tparam renderer_t A type conforming to the @ref Rendering::IRenderer interface
	 * @tparam meshbuilder_t A type conforming to the @ref Rendering::IMeshBuilder interface
	 *
	 * @todo Document parameters
	 *
	 * @return The created object
	 */
	template <class renderer_t, class meshbuilder_t>
	SceneObject* createSceneObject(
		Engine*												 with_engine,
		std::string											 with_pipeline_program,
		const std::vector<Rendering::RendererSupplyData>&	 renderer_supply_data,
		const std::vector<Rendering::MeshBuilderSupplyData>& meshbuilder_supply_data
	)
		requires(
			(std::is_same_v<renderer_t, Rendering::Renderer2D> && meshbuilder_t::supports_2d) ||
			(std::is_same_v<renderer_t, Rendering::Renderer3D> && meshbuilder_t::supports_3d)
		)
	{
		if (with_pipeline_program.empty())
		{
			throw ENGINE_EXCEPTION("Cannot CreateSceneObject without a pipeline! (was empty)");
		}

		auto* with_builder = new meshbuilder_t(with_engine);

		for (const auto& supply : meshbuilder_supply_data)
		{
			with_builder->supplyData(supply);
		}

		auto* with_renderer =
			new renderer_t(with_engine, with_builder, with_pipeline_program.c_str());

		for (const auto& supply : renderer_supply_data)
		{
			with_renderer->supplyData(supply);
		}

		auto* object = new SceneObject(with_engine, with_renderer, with_builder);

		return object;
	}

	/**
	 * Callable type that creates an object.
	 *
	 * For implementation details see @ref createSceneObject.
	 */
	using object_factory_t = std::function<
		SceneObject*(Engine*, std::string, const std::vector<Rendering::RendererSupplyData>&, const std::vector<Rendering::MeshBuilderSupplyData>&)>;

	/**
	 * Get an object factory capable of creating the desired object.
	 *
	 * @param with_renderer Desired Renderer type.
	 * @param with_mesh_builder Desired MeshBuilder type.
	 *
	 * @return Callable that performs the object creation.
	 */
	object_factory_t getSceneObjectFactory(
		EnumStringConvertor<RendererType>	 with_renderer,
		EnumStringConvertor<MeshBuilderType> with_mesh_builder
	);

	SceneObject* instantiateObjectTemplate(Engine* with_engine, ObjectTemplate& from_template);

	using supply_data_value_t = std::variant<std::monostate, void*, std::string>;

	Rendering::RendererSupplyData generateRendererSupplyData(
		EnumStringConvertor<Rendering::RendererSupplyData::Type> of_type,
		supply_data_value_t										 with_value
	);

	Rendering::MeshBuilderSupplyData generateMeshBuilderSupplyData(
		EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> of_type,
		supply_data_value_t											with_value
	);

} // namespace Engine::Factories::ObjectFactory
