#pragma once

#include "Assets/Mesh.hpp"

#include "Object.hpp"
#include "SceneManager.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Font;
} // namespace Engine::Rendering

// TODO: Move to Factories namespace?
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
	);

	Object* CreateTextObject(
		const std::weak_ptr<SceneManager>& scene_manager,
		double x,
		double y,
		double z,
		int size,
		std::string text,
		const std::shared_ptr<Rendering::Font>& font
	);

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
		const std::shared_ptr<::Engine::Rendering::Mesh>& mesh
	);
} // namespace Engine::ObjectFactory
