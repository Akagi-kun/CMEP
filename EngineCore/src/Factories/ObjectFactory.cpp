#include "Factories/ObjectFactory.hpp"

#include "Rendering/AxisMeshBuilder.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Renderer2D.hpp"
#include "Rendering/SpriteMeshBuilder.hpp"
#include "Rendering/TextMeshBuilder.hpp"

#include "EnumStringConvertor.hpp"

#include <cassert>
#include <stdexcept>

namespace Engine
{
	using namespace Factories::ObjectFactory;

	template <>
	EnumStringConvertor<RendererType>::map_type EnumStringConvertor<RendererType>::type_map = {
		{"generic_2d", value_type::GENERIC_2D},
		{"generic_3d", value_type::GENERIC_3D},
	};

	template <>
	EnumStringConvertor<MeshBuilderType>::map_type EnumStringConvertor<MeshBuilderType>::type_map = {
		{"sprite", value_type::SPRITE},
		{"text", value_type::TEXT},
		{"axis", value_type::AXIS},
	};
} // namespace Engine

namespace Engine::Factories::ObjectFactory
{
	/* Object* CreateGeneric3DObject(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Rendering::Mesh>& mesh)
	{
		Engine* engine = scene->GetOwnerEngine();

		auto* object = new Object(engine);

		Rendering::IRenderer* with_renderer = new Rendering::MeshRenderer(engine, nullptr);
		with_renderer->SupplyData({Rendering::RendererSupplyDataType::MESH, mesh});

		object->SetRenderer(with_renderer);

		return object;
	} */

	std::function<Object*(Engine*, std::string, const std::vector<Rendering::RendererSupplyData>&)>
	GetSceneObjectFactory(
		EnumStringConvertor<RendererType> with_renderer,
		EnumStringConvertor<MeshBuilderType> with_mesh_builder
	)
	{
		switch (with_renderer)
		{
			case RendererType::GENERIC_2D:
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
					case MeshBuilderType::AXIS:
					{
						return CreateSceneObject<Rendering::Renderer2D, Rendering::AxisMeshBuilder>;
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

} // namespace Engine::Factories::ObjectFactory
