#pragma once

#include "Assets/Mesh.hpp"

#include "Engine.hpp"
#include "Object.hpp"
#include "Scene.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Font;
} // namespace Engine::Rendering

// TODO: Move to Factories namespace?
namespace Engine::ObjectFactory
{
	Object* CreateGeneric3DObject(
		const std::shared_ptr<Scene>& scene,
		const std::shared_ptr<::Engine::Rendering::Mesh>& mesh
	);

	template <class TRenderer, class TMeshBuilder>
	Object* CreateSceneObject(Engine* with_engine, const std::vector<Rendering::RendererSupplyData>& with_supply_data)
	{
		auto* object = new Object(with_engine);

		auto* with_builder	= new TMeshBuilder(with_engine, with_engine->GetRenderingEngine());
		auto* with_renderer = new TRenderer(with_engine, with_builder);

		for (const auto& supply : with_supply_data)
		{
			with_renderer->SupplyData(supply);
		}

		object->SetRenderer(with_renderer);

		return object;
	}
} // namespace Engine::ObjectFactory
