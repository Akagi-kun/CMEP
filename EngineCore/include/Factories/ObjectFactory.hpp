#pragma once

#include "Assets/Mesh.hpp"

#include "Object.hpp"
#include "Scene.hpp"
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
		const std::shared_ptr<Scene>& scene,
		const std::shared_ptr<::Engine::Rendering::Texture>& sprite
	);

	Object* CreateTextObject(
		const std::shared_ptr<Scene>& scene,
		std::string text,
		const std::shared_ptr<Rendering::Font>& font
	);

	Object* CreateGeneric3DObject(
		const std::shared_ptr<Scene>& scene,
		const std::shared_ptr<::Engine::Rendering::Mesh>& mesh
	);
} // namespace Engine::ObjectFactory
