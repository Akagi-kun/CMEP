#include "Factories/ObjectFactory.hpp"

#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"
#include "Rendering/MeshBuilders/GeneratorMeshBuilder.hpp"
#include "Rendering/MeshBuilders/SpriteMeshBuilder.hpp"
#include "Rendering/MeshBuilders/TextMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/SupplyData.hpp"

#include "EnumStringConvertor.hpp"
#include "Exception.hpp"
#include "InternalEngineObject.hpp"
#include "SceneObject.hpp"

#include <cassert>
#include <format>
#include <memory>
#include <string>

namespace Engine::Factories::ObjectFactory
{
	object_factory_t getSceneObjectFactory(
		EnumStringConvertor<RendererType>	 with_renderer,
		EnumStringConvertor<MeshBuilderType> with_mesh_builder
	)
	{
		switch (with_renderer)
		{
			case RendererType::RENDERER_2D:
			{
				using r_type = Rendering::Renderer2D;
				switch (with_mesh_builder)
				{
					case MeshBuilderType::SPRITE:
					{
						using mb_type = Rendering::SpriteMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::TEXT:
					{
						using mb_type = Rendering::TextMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					}
					// Axis is not compatible with 2D rendering
					/* case MeshBuilderType::AXIS:
					{
						using mb_type = Rendering::AxisMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					} */
					default:
					{
						throw ENGINE_EXCEPTION("Invalid mesh builder type!");
					}
				}
			}
			case RendererType::RENDERER_3D:
			{
				using r_type = Rendering::Renderer3D;
				switch (with_mesh_builder)
				{
					case MeshBuilderType::SPRITE:
					{
						using mb_type = Rendering::SpriteMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::TEXT:
					{
						using mb_type = Rendering::TextMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::AXIS:
					{
						using mb_type = Rendering::AxisMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::GENERATOR:
					{
						using mb_type = Rendering::GeneratorMeshBuilder;

						return createSceneObject<r_type, mb_type>;
					}
					default:
					{
						throw ENGINE_EXCEPTION("Invalid mesh builder type!");
					}
				}
			}
			default:
			{
				throw ENGINE_EXCEPTION("Invalid renderer type!");
			}
		}
	}

	SceneObject* instantiateObjectTemplate(Engine* with_engine, ObjectTemplate& from_template)
	{
		const auto& factory =
			getSceneObjectFactory(from_template.with_renderer, from_template.with_mesh_builder);

		return factory(
			with_engine,
			from_template.with_shader,
			from_template.renderer_supply_list,
			from_template.meshbuilder_supply_list
		);
	}

	Rendering::RendererSupplyData generateRendererSupplyData(
		EnumStringConvertor<Rendering::RendererSupplyData::Type> of_type,
		supply_data_value_t										 with_value
	)
	{
		EXCEPTION_ASSERT(with_value.index() != 0, "Invalid supply data value passed!");

		switch (of_type)
		{
			case Rendering::RendererSupplyData::Type::TEXTURE:
			{
				return {
					of_type,
					*static_cast<std::weak_ptr<Rendering::Texture>*>(std::get<void*>(with_value))
				};
			}
			case Rendering::RendererSupplyData::Type::FONT:
			{
				return {
					of_type,
					*static_cast<std::weak_ptr<Rendering::Font>*>(std::get<void*>(with_value))
				};
			}
			default:
			{
				throw ENGINE_EXCEPTION(std::format(
					"RendererSupplyDataType is unknown, invalid or missing! Type: {}",
					static_cast<std::string>(of_type)
				));
			}
		}
	}

	Rendering::MeshBuilderSupplyData generateMeshBuilderSupplyData(
		EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> of_type,
		supply_data_value_t											with_value
	)
	{
		EXCEPTION_ASSERT(with_value.index() != 0, "Invalid supply data value passed!");

		switch (of_type)
		{
			case Rendering::MeshBuilderSupplyData::Type::TEXT:
			{
				return {of_type, std::get<std::string>(with_value)};
			}
			case Rendering::MeshBuilderSupplyData::Type::GENERATOR:
			{
				auto* cdata_ptr = static_cast<Rendering::GeneratorData*>(std::get<void*>(with_value)
				);

				return {of_type, *cdata_ptr};
			}
			default:
			{
				throw ENGINE_EXCEPTION(std::format(
					"MeshBuilderSupplyData is unknown, invalid or missing! Type: {}",
					static_cast<std::string>(of_type)
				));
			}
		}
	}
} // namespace Engine::Factories::ObjectFactory
