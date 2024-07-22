#include "Factories/ObjectFactory.hpp"

#include "Rendering/AxisMeshBuilder.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Renderer2D.hpp"
#include "Rendering/Renderer3D.hpp"
#include "Rendering/SpriteMeshBuilder.hpp"
#include "Rendering/TextMeshBuilder.hpp"

#include "EnumStringConvertor.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <stdexcept>

namespace Engine::Factories::ObjectFactory
{
	std::function<
		Object*(Engine*, std::string, const std::vector<Rendering::RendererSupplyData>&, EnumStringConvertor<VkPrimitiveTopology>)>
	GetSceneObjectFactory(
		EnumStringConvertor<RendererType> with_renderer,
		EnumStringConvertor<MeshBuilderType> with_mesh_builder
	)
	{
		switch (with_renderer)
		{
			case RendererType::RENDERER_2D:
			{
				switch (with_mesh_builder)
				{
					case MeshBuilderType::SPRITE:
					{
						return CreateSceneObject<Rendering::Renderer2D, Rendering::SpriteMeshBuilder>;
					}
					case MeshBuilderType::TEXT:
					{
						return CreateSceneObject<Rendering::Renderer2D, Rendering::TextMeshBuilder>;
					}
					// case MeshBuilderType::AXIS:
					//{
					//	return CreateSceneObject<Rendering::Renderer2D, Rendering::AxisMeshBuilder>;
					// }
					default:
					{
						throw std::invalid_argument("Invalid mesh builder type!");
					}
				}
			}
			case RendererType::RENDERER_3D:
			{
				switch (with_mesh_builder)
				{
					case MeshBuilderType::SPRITE:
					{
						return CreateSceneObject<Rendering::Renderer3D, Rendering::SpriteMeshBuilder>;
					}
					case MeshBuilderType::TEXT:
					{
						return CreateSceneObject<Rendering::Renderer3D, Rendering::TextMeshBuilder>;
					}
					case MeshBuilderType::AXIS:
					{
						return CreateSceneObject<Rendering::Renderer3D, Rendering::AxisMeshBuilder>;
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

		return factory(
			with_engine,
			from_template.with_shader,
			from_template.supply_list,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		);
	}

} // namespace Engine::Factories::ObjectFactory
