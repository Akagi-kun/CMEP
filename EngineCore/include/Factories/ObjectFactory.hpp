#pragma once

#include "EnumStringConvertor.hpp"
#include "Object.hpp"
#include "vulkan/vulkan_core.h"

#include <stdexcept>
#include <string>

namespace Engine
{
	namespace Factories::ObjectFactory
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

			TEXT   = 1,
			SPRITE = 2,
			MESH   = 3,
			AXIS   = 4,

			MAX_ENUM = 0XFF
		};
	} // namespace Factories::ObjectFactory

	// template struct EnumStringConvertor<Factories::ObjectFactory::RendererType>;

	namespace Factories::ObjectFactory
	{
		struct ObjectTemplate
		{
			EnumStringConvertor<RendererType> with_renderer		   = RendererType::MIN_ENUM;
			EnumStringConvertor<MeshBuilderType> with_mesh_builder = MeshBuilderType::MIN_ENUM;
			std::string with_shader;
			std::vector<Rendering::RendererSupplyData> supply_list;
		};

		template <class TRenderer, class TMeshBuilder>
		Object* CreateSceneObject(
			Engine* with_engine,
			std::string with_pipeline_program,
			const std::vector<Rendering::RendererSupplyData>& with_supply_data,
			EnumStringConvertor<VkPrimitiveTopology> with_primitives = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		)
		{
			if (with_pipeline_program.empty())
			{
				throw std::invalid_argument("Cannot CreateSceneObject without a pipeline! (was empty)");
			}

			auto* object = new Object(with_engine);

			auto* with_builder = new TMeshBuilder(with_engine);
			auto* with_renderer =
				new TRenderer(with_engine, with_builder, with_pipeline_program.c_str(), with_primitives);

			for (const auto& supply : with_supply_data)
			{
				with_renderer->SupplyData(supply);
			}

			object->SetRenderer(with_renderer);

			return object;
		}

		std::function<
			Object*(Engine*, std::string, const std::vector<Rendering::RendererSupplyData>&, EnumStringConvertor<VkPrimitiveTopology>)>
		GetSceneObjectFactory(
			EnumStringConvertor<RendererType> with_renderer,
			EnumStringConvertor<MeshBuilderType> with_mesh_builder
		);

		Object* InstantiateObjectTemplate(Engine* with_engine, ObjectTemplate& from_template);

	} // namespace Factories::ObjectFactory
} // namespace Engine
