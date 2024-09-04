#include "Factories/ObjectFactory.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"
#include "Rendering/MeshBuilders/GeneratorMeshBuilder.hpp"
#include "Rendering/MeshBuilders/SpriteMeshBuilder.hpp"
#include "Rendering/MeshBuilders/TextMeshBuilder.hpp"
#include "Rendering/SupplyData.hpp"

#include "EnumStringConvertor.hpp"
#include "Exception.hpp"

#include <cassert>
#include <stdexcept>

namespace Engine::Factories::ObjectFactory
{
	object_factory_t GetSceneObjectFactory(
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
						static_assert(mb_type::supports_2d);

						return CreateSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::TEXT:
					{
						using mb_type = Rendering::TextMeshBuilder;
						static_assert(mb_type::supports_2d);

						return CreateSceneObject<r_type, mb_type>;
					}
					// Axis is not compatible with 2D rendering
					/* case MeshBuilderType::AXIS:
					{
						using mb_type = Rendering::AxisMeshBuilder;
						static_assert(mb_type::supports_2d); // Fails

						return CreateSceneObject<r_type, mb_type>;
					} */
					default:
					{
						throw std::invalid_argument("Invalid mesh builder type!");
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
						static_assert(mb_type::supports_3d);

						return CreateSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::TEXT:
					{
						using mb_type = Rendering::TextMeshBuilder;
						static_assert(mb_type::supports_3d);

						return CreateSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::AXIS:
					{
						using mb_type = Rendering::AxisMeshBuilder;
						static_assert(mb_type::supports_3d);

						return CreateSceneObject<r_type, mb_type>;
					}
					case MeshBuilderType::GENERATOR:
					{
						using mb_type = Rendering::GeneratorMeshBuilder;
						static_assert(mb_type::supports_3d);

						return CreateSceneObject<r_type, mb_type>;
					}
					default:
					{
						throw std::invalid_argument("Invalid mesh builder type!");
					}
				}
			}
			default:
			{
				throw std::invalid_argument("Invalid renderer type!");
			}
		}
	}

	Object* InstantiateObjectTemplate(Engine* with_engine, ObjectTemplate& from_template)
	{
		const auto& factory =
			GetSceneObjectFactory(from_template.with_renderer, from_template.with_mesh_builder);

		return factory(
			with_engine,
			from_template.with_shader,
			from_template.renderer_supply_list,
			from_template.meshbuilder_supply_list
		);
	}

	Rendering::RendererSupplyData GenerateRendererSupplyData(
		EnumStringConvertor<Rendering::RendererSupplyData::Type> of_type,
		valid_value_t											 with_value
	)
	{
		ENGINE_EXCEPTION_ON_ASSERT(with_value.index() != 0, "Invalid supply data value passed!")

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

	Rendering::MeshBuilderSupplyData GenerateMeshBuilderSupplyData(
		EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> of_type,
		valid_value_t												with_value
	)
	{
		ENGINE_EXCEPTION_ON_ASSERT(with_value.index() != 0, "Invalid supply data value passed!")

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
