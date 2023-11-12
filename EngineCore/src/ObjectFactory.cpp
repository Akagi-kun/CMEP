#include <assert.h>

#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Font.hpp"
#include "ObjectFactory.hpp"
#include "glm/vec3.hpp"

namespace Engine::ObjectFactory
{
	Object* CreateSpriteObject(std::weak_ptr<GlobalSceneManager> scene_manager, double x, double y, double sizex, double sizey, std::shared_ptr<::Engine::Rendering::Texture> sprite)
	{
		assert(sprite != nullptr);
		if(auto locked_scene_manager = scene_manager.lock())
		{
			Object* object = new Object();
			object->renderer = new Rendering::SpriteRenderer(locked_scene_manager->owner_engine);
			object->Translate(glm::vec3(x, y, 0));
			object->Scale(glm::vec3(sizex, sizey, 0));
			((Rendering::SpriteRenderer*)object->renderer)->scene_manager = scene_manager;
			((Rendering::SpriteRenderer*)object->renderer)->UpdateTexture(sprite);
			((Rendering::SpriteRenderer*)object->renderer)->UpdateMesh();
			return object;
		}
		return nullptr;
	}

	Object* CreateTextObject(std::weak_ptr<GlobalSceneManager> scene_manager, double x, double y, int size, std::string text, ::Engine::Rendering::Font* font)
	{
		assert(font != nullptr);
		if(auto locked_scene_manager = scene_manager.lock())
		{
			Object* object = new Object();
			object->renderer = new Rendering::TextRenderer(locked_scene_manager->owner_engine);
			object->Translate(glm::vec3(x, y, 0));
			object->Scale(glm::vec3(size, size, 0));
			((Rendering::TextRenderer*)object->renderer)->scene_manager = scene_manager;
			((Rendering::TextRenderer*)object->renderer)->UpdateFont(font);
			((Rendering::TextRenderer*)object->renderer)->UpdateText(std::move(text));
			((Rendering::TextRenderer*)object->renderer)->UpdateMesh();
			
			return object;
		}
		return nullptr;
	}

	Object* CreateGeneric3DObject(std::weak_ptr<GlobalSceneManager> scene_manager, double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, std::shared_ptr<::Engine::Rendering::Mesh> mesh)
	{
		if(auto locked_scene_manager = scene_manager.lock())
		{
			Object* object = new Object();
			object->renderer = new Rendering::MeshRenderer(locked_scene_manager->owner_engine);
			object->Translate(glm::vec3(x, y, z));
			object->Scale(glm::vec3(sizex, sizey, sizez));
			object->Rotate(glm::vec3(rotx, roty, rotz));
			((Rendering::MeshRenderer*)object->renderer)->scene_manager = scene_manager;
			((Rendering::MeshRenderer*)object->renderer)->AssignMesh(mesh);
			return object;
		}
		return nullptr;
	}
}