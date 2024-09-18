#pragma once

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Transform.hpp"

#include "InternalEngineObject.hpp"

#include <vector>

namespace Engine
{
	class Object final : public InternalEngineObject
	{
	public:
		Object(
			Engine*					 with_engine,
			Rendering::IRenderer*	 with_renderer,
			Rendering::IMeshBuilder* with_mesh_builder
		);
		~Object() noexcept;

		void screenSizeInform(Rendering::ScreenSize with_screen_size);

		/**
		 * @todo relative functions?
		 */
		void setPosition(glm::vec3 with_pos);
		void setSize(glm::vec3 with_size);
		void setRotation(glm::vec3 with_rotation);

		[[nodiscard]] Rendering::IRenderer* getRenderer()
		{
			return renderer;
		}
		[[nodiscard]] Rendering::IMeshBuilder* getMeshBuilder()
		{
			return mesh_builder;
		}

		[[nodiscard]] glm::vec3 getPosition() const noexcept;
		[[nodiscard]] glm::vec3 getSize() const noexcept;
		[[nodiscard]] glm::vec3 getRotation() const noexcept;

		void setParentTransform(Rendering::Transform with_parent_transform);

		void addChild(Object* with_child);
		void removeChildren();
		void setParent(Object* with_parent);

	private:
		Rendering::Transform transform;
		// Initialize parent transform so that the object renders without parent properly
		Rendering::Transform parent_transform = {
			glm::vec3(0),
			glm::vec3(1, 1, 1),
			glm::vec3(0)
		};
		Rendering::ScreenSize screen;

		Object* parent;

		std::vector<Object*> children;

		Rendering::IRenderer*	 renderer	  = nullptr;
		Rendering::IMeshBuilder* mesh_builder = nullptr;

		void updateRenderer();
	};
} // namespace Engine
