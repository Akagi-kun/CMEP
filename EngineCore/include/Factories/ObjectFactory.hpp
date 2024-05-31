#pragma once

#include "Rendering/Mesh.hpp"
#include "Object.hpp"
#include "SceneManager.hpp"
#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Font;
}

// TODO: Move to Factories namespace?
namespace Engine::ObjectFactory
{
	CMEP_EXPORT Object* CreateSpriteObject(std::weak_ptr<SceneManager> scene_manager, double x, double y, double z, double sizex, double sizey, std::shared_ptr<::Engine::Rendering::Texture> sprite);
	CMEP_EXPORT Object* CreateTextObject(std::weak_ptr<SceneManager> scene_manager, double x, double y, double z, int size, std::string text, ::Engine::Rendering::Font* font);
	CMEP_EXPORT Object* CreateGeneric3DObject(std::weak_ptr<SceneManager> scene_manager, double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, std::shared_ptr<::Engine::Rendering::Mesh> mesh);
}