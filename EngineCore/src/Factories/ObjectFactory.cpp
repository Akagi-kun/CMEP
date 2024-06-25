#include "Factories/ObjectFactory.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/TextRenderer.hpp"

#include "IModule.hpp"
// #include "glm/vec3.hpp"

#include <cassert>
#include <memory>

namespace Engine::ObjectFactory
{
	Object* CreateSpriteObject(
		const std::weak_ptr<SceneManager>& scene_manager,
		const std::shared_ptr<::Engine::Rendering::Texture>& sprite
	)
	{
		assert(sprite != nullptr);
		if (auto locked_scene_manager = scene_manager.lock())
		{
			auto* object = new Object();

			Rendering::IRenderer* with_renderer = new Rendering::SpriteRenderer(locked_scene_manager->GetOwnerEngine());
			with_renderer->scene_manager		= scene_manager;

			ModuleMessage texture_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXTURE, sprite}
			};
			with_renderer->Communicate(texture_supply_message);

			object->AddModule(ModuleType::RENDERER, with_renderer);

			return object;
		}
		return nullptr;
	}

	Object* CreateTextObject(
		const std::weak_ptr<SceneManager>& scene_manager,
		std::string text,
		const std::shared_ptr<Rendering::Font>& font
	)
	{
		assert(font != nullptr);
		if (auto locked_scene_manager = scene_manager.lock())
		{
			auto* object = new Object();

			Rendering::IRenderer* with_renderer = new Rendering::TextRenderer(locked_scene_manager->GetOwnerEngine());
			with_renderer->scene_manager		= scene_manager;

			ModuleMessage font_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::FONT, font}
			};
			with_renderer->Communicate(font_supply_message);

			ModuleMessage text_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXT, std::move(text)}
			};
			with_renderer->Communicate(text_supply_message);

			object->AddModule(ModuleType::RENDERER, with_renderer);

			return object;
		}
		return nullptr;
	}

	Object* CreateGeneric3DObject(
		const std::weak_ptr<SceneManager>& scene_manager,
		const std::shared_ptr<Rendering::Mesh>& mesh
	)
	{
		if (auto locked_scene_manager = scene_manager.lock())
		{
			auto* object = new Object();

			Rendering::IRenderer* with_renderer = new Rendering::MeshRenderer(locked_scene_manager->GetOwnerEngine());
			with_renderer->scene_manager		= scene_manager;

			ModuleMessage mesh_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::MESH, mesh}
			};
			with_renderer->Communicate(mesh_supply_message);

			object->AddModule(ModuleType::RENDERER, with_renderer);
			return object;
		}
		return nullptr;
	}
} // namespace Engine::ObjectFactory
