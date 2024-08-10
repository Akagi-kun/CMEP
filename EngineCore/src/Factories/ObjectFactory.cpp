#include "Factories/ObjectFactory.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"
#include "Rendering/MeshBuilders/GeneratorMeshBuilder.hpp"
#include "Rendering/MeshBuilders/SpriteMeshBuilder.hpp"
#include "Rendering/MeshBuilders/TextMeshBuilder.hpp"
#include "Rendering/SupplyData.hpp"

#include "EnumStringConvertor.hpp"
#include "KVPairHelper.hpp"

#include <any>
#include <cassert>
#include <stdexcept>

namespace Engine::Factories::ObjectFactory
{
	std::function<Object*(Engine*, std::string, const std::vector<Rendering::RendererSupplyData>&)>
	GetSceneObjectFactory(
		EnumStringConvertor<RendererType> with_renderer,
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
		const auto& factory = GetSceneObjectFactory(from_template.with_renderer, from_template.with_mesh_builder);

		return factory(with_engine, from_template.with_shader, from_template.supply_list);
	}

	void PushSupplyData(
		AssetManager* asset_manager,
		std::vector<Rendering::RendererSupplyData>& into_vector,
		EnumStringConvertor<Rendering::RendererSupplyDataType> of_type,
		std::any with_value
	)
	{
		if (with_value.has_value())
		{
			switch (of_type)
			{
				case Rendering::RendererSupplyDataType::TEXTURE:
				{
					into_vector.emplace_back(
						of_type,
						asset_manager->GetTexture(std::any_cast<std::string>(with_value))
					);
					return;
				}
				case Rendering::RendererSupplyDataType::FONT:
				{
					into_vector.emplace_back(of_type, asset_manager->GetFont(std::any_cast<std::string>(with_value)));
					return;
				}
				case Rendering::RendererSupplyDataType::TEXT:
				{
					into_vector.emplace_back(of_type, std::any_cast<std::string>(with_value));
					return;
				}
				case Rendering::RendererSupplyDataType::GENERATOR_SCRIPT:
				{
					into_vector.emplace_back(
						of_type,
						asset_manager->GetLuaScript(std::any_cast<std::string>(with_value))
					);
					return;
				}
				case Rendering::RendererSupplyDataType::GENERATOR_SUPPLIER:
				{
					const auto [key, val] = Utility::SplitKVPair(std::any_cast<std::string>(with_value), "/");

					into_vector.emplace_back(
						of_type,
						Rendering::GeneratorSupplierData{asset_manager->GetLuaScript(key), val}
					);
					return;
				}
				default:
				{
					throw std::invalid_argument("RendererSupplyDataType is unknown, invalid or missing!");
				}
			}
		}

		throw std::invalid_argument("Cannot PushSupplyData without a value to push! (std::any::has_value() is false)");
	}

} // namespace Engine::Factories::ObjectFactory
