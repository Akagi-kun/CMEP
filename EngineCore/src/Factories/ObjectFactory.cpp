#include "Factories/ObjectFactory.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SpriteMeshBuilder.hpp"
#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/TextMeshBuilder.hpp"
#include "Rendering/TextRenderer.hpp"

#include "Engine.hpp"

// #include "glm/vec3.hpp"

#include <cassert>
#include <memory>

namespace Engine::ObjectFactory
{
	Object* CreateSpriteObject(
		const std::shared_ptr<Scene>& scene,
		const std::shared_ptr<::Engine::Rendering::Texture>& sprite
	)
	{
		assert(sprite != nullptr);
		// if (auto locked_scene_manager = scene_manager.lock())
		//{
		Engine* engine = scene->GetOwnerEngine(); // locked_scene_manager->GetOwnerEngine();

		auto* object = new Object(engine);

		Rendering::IMeshBuilder* with_builder = new Rendering::SpriteMeshBuilder(engine, engine->GetRenderingEngine());

		Rendering::IRenderer* with_renderer = new Rendering::SpriteRenderer(engine, with_builder);
		// with_renderer->scene_manager		= scene_manager;

		// Rendering::RendererSupplyData texture_supply = {Rendering::RendererSupplyDataType::TEXTURE, sprite};

		with_renderer->SupplyData({Rendering::RendererSupplyDataType::TEXTURE, sprite});

		object->SetRenderer(with_renderer);

		return object;
		//}
		// return nullptr;
	}

	Object* CreateTextObject(
		const std::shared_ptr<Scene>& scene,
		std::string text,
		const std::shared_ptr<Rendering::Font>& font
	)
	{
		assert(font != nullptr);
		// if (auto locked_scene_manager = scene_manager.lock())
		//{
		Engine* engine = scene->GetOwnerEngine();

		auto* object = new Object(engine);

		Rendering::IMeshBuilder* with_builder = new Rendering::TextMeshBuilder(engine, engine->GetRenderingEngine());

		Rendering::IRenderer* with_renderer = new Rendering::TextRenderer(engine, with_builder);
		// with_renderer->scene_manager		= scene_manager;

		// Rendering::RendererSupplyData font_supply = {Rendering::RendererSupplyDataType::FONT, font};
		with_renderer->SupplyData({Rendering::RendererSupplyDataType::FONT, font});

		// Rendering::RendererSupplyData text_supply = {Rendering::RendererSupplyDataType::TEXT, std::move(text)};
		with_renderer->SupplyData({Rendering::RendererSupplyDataType::TEXT, std::move(text)});

		object->SetRenderer(with_renderer);

		return object;
		//}
		// return nullptr;
	}

	Object* CreateGeneric3DObject(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Rendering::Mesh>& mesh)
	{
		// if (auto locked_scene_manager = scene_manager.lock())
		//{
		Engine* engine = scene->GetOwnerEngine();

		auto* object = new Object(engine);

		Rendering::IRenderer* with_renderer = new Rendering::MeshRenderer(engine);
		// with_renderer->scene_manager		= scene_manager;

		// Rendering::RendererSupplyData mesh_supply = {Rendering::RendererSupplyDataType::MESH, mesh};

		with_renderer->SupplyData({Rendering::RendererSupplyDataType::MESH, mesh});

		object->SetRenderer(with_renderer);

		return object;
		//}
		// return nullptr;
	}
} // namespace Engine::ObjectFactory
