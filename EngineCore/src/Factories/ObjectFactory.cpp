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
		double x,
		double y,
		double z,
		double sizex,
		double sizey,
		const std::shared_ptr<::Engine::Rendering::Texture>& sprite
	)
	{
		assert(sprite != nullptr);
		if (auto locked_scene_manager = scene_manager.lock())
		{
			auto* object = new Object();

			// object->renderer = new Rendering::SpriteRenderer(locked_scene_manager->GetOwnerEngine());
			Rendering::IRenderer* with_renderer = new Rendering::SpriteRenderer(locked_scene_manager->GetOwnerEngine());
			with_renderer->scene_manager		= scene_manager;

			object->Translate(glm::vec3(x, y, z));
			object->Scale(glm::vec3(sizex, sizey, 0));

			// object->renderer->scene_manager = scene_manager;

			ModuleMessage texture_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXTURE, sprite}
			};
			// Rendering::RendererSupplyData texture_supply(Rendering::RendererSupplyDataType::TEXTURE, sprite);
			//  object->renderer->SupplyData(texture_supply);
			// with_renderer->SupplyData(texture_supply);
			with_renderer->Communicate(texture_supply_message);

			//((Rendering::SpriteRenderer*)object->renderer)->UpdateMesh();
			// object->renderer->UpdateMesh();

			object->AddModule(ModuleType::RENDERER, with_renderer);
			// auto* old_renderer = object->AssignRenderer(with_renderer);
			// assert(old_renderer == nullptr);

			return object;
		}
		return nullptr;
	}

	Object* CreateTextObject(
		const std::weak_ptr<SceneManager>& scene_manager,
		double x,
		double y,
		double z,
		int size,
		std::string text,
		const std::shared_ptr<Rendering::Font>& font
	)
	{
		assert(font != nullptr);
		if (auto locked_scene_manager = scene_manager.lock())
		{
			auto* object = new Object();

			// object->renderer = new Rendering::TextRenderer(locked_scene_manager->GetOwnerEngine());

			Rendering::IRenderer* with_renderer = new Rendering::TextRenderer(locked_scene_manager->GetOwnerEngine());
			with_renderer->scene_manager		= scene_manager;

			object->Translate(glm::vec3(x, y, z));
			object->Scale(glm::vec3(size, size, 0));

			// Rendering::RendererSupplyData font_supply(Rendering::RendererSupplyDataType::FONT, font);

			ModuleMessage font_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::FONT, font}
			};
			// Rendering::RendererSupplyData font_supply(Rendering::RendererSupplyDataType::FONT, font);
			with_renderer->Communicate(font_supply_message);
			// with_renderer->SupplyData(font_supply);

			ModuleMessage text_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXT, std::move(text)}
			};
			// Rendering::RendererSupplyData text_supply(Rendering::RendererSupplyDataType::TEXT, text);
			with_renderer->Communicate(text_supply_message);
			// with_renderer->SupplyData(text_supply);

			// object->renderer->UpdateMesh();

			object->AddModule(ModuleType::RENDERER, with_renderer);
			// auto* old_renderer = object->AssignRenderer(with_renderer);
			// assert(old_renderer == nullptr);

			return object;
		}
		return nullptr;
	}

	Object* CreateGeneric3DObject(
		const std::weak_ptr<SceneManager>& scene_manager,
		double x,
		double y,
		double z,
		double sizex,
		double sizey,
		double sizez,
		double rotx,
		double roty,
		double rotz,
		const std::shared_ptr<Rendering::Mesh>& mesh
	)
	{
		if (auto locked_scene_manager = scene_manager.lock())
		{
			auto* object = new Object();

			object->Translate(glm::vec3(x, y, z));
			object->Scale(glm::vec3(sizex, sizey, sizez));
			object->Rotate(glm::vec3(rotx, roty, rotz));

			Rendering::IRenderer* with_renderer = new Rendering::MeshRenderer(locked_scene_manager->GetOwnerEngine());
			with_renderer->scene_manager		= scene_manager;

			ModuleMessage mesh_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::MESH, mesh}
			};
			with_renderer->Communicate(mesh_supply_message);
			// Rendering::RendererSupplyData mesh_supply(Rendering::RendererSupplyDataType::MESH, mesh);
			// with_renderer->SupplyData(mesh_supply);

			object->AddModule(ModuleType::RENDERER, with_renderer);
			// auto* old_renderer = object->AssignRenderer(with_renderer);
			// assert(old_renderer == nullptr);

			return object;
		}
		return nullptr;
	}
} // namespace Engine::ObjectFactory
