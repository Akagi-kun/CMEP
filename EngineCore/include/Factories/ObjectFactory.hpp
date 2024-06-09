#pragma once

#include "Object.hpp"
#include "PlatformSemantics.hpp"
#include "Rendering/Mesh.hpp"
#include "SceneManager.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Font;
} // namespace Engine::Rendering

// TODO: Move to Factories namespace?
namespace Engine::ObjectFactory
{
	CMEP_EXPORT Object* CreateSpriteObject(
		std::weak_ptr<SceneManager> scene_manager,
		double x,
		double y,
		double z,
		double sizex,
		double sizey,
		std::shared_ptr<Rendering::Texture> sprite
	);
	
	CMEP_EXPORT Object* CreateTextObject(
		std::weak_ptr<SceneManager> scene_manager,
		double x,
		double y,
		double z,
		int size,
		std::string text,
		std::shared_ptr<Rendering::Font> font
	);

	CMEP_EXPORT Object* CreateGeneric3DObject(
		std::weak_ptr<SceneManager> scene_manager,
		double x,
		double y,
		double z,
		double sizex,
		double sizey,
		double sizez,
		double rotx,
		double roty,
		double rotz,
		std::shared_ptr<::Engine::Rendering::Mesh> mesh
	);
} // namespace Engine::ObjectFactory