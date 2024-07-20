#pragma once

#include "Assets/Mesh.hpp"

#include "Object.hpp"
#include "Scene.hpp"

#include <stdexcept>

// TODO: Move to Factories namespace?
namespace Engine::ObjectFactory
{
	Object* CreateGeneric3DObject(
		const std::shared_ptr<Scene>& scene,
		const std::shared_ptr<::Engine::Rendering::Mesh>& mesh
	);

	template <class TRenderer, class TMeshBuilder>
	Object* CreateSceneObject(
		Engine* with_engine,
		const std::vector<Rendering::RendererSupplyData>& with_supply_data,
		std::string with_pipeline_program
	)
	{
		if (with_pipeline_program.empty())
		{
			throw std::invalid_argument("Cannot CreateSceneObject without a pipeline! (was empty)");
		}

		auto* object = new Object(with_engine);

		auto* with_builder	= new TMeshBuilder(with_engine);
		auto* with_renderer = new TRenderer(with_engine, with_builder, with_pipeline_program.c_str());

		for (const auto& supply : with_supply_data)
		{
			with_renderer->SupplyData(supply);
		}

		object->SetRenderer(with_renderer);

		return object;
	}
} // namespace Engine::ObjectFactory
