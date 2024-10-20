#pragma once

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Transform.hpp"

#include "InternalEngineObject.hpp"

#include <vector>

namespace Engine
{
	class SceneObject final : public InternalEngineObject
	{
	public:
		SceneObject(
			Engine*					 with_engine,
			Rendering::IRenderer*	 with_renderer,
			Rendering::IMeshBuilder* with_mesh_builder
		);
		~SceneObject() noexcept;

		/**
		 * @todo relative functions?
		 */
		void setPosition(const glm::vec3& with_pos);
		void setSize(const glm::vec3& with_size);
		void setRotation(const glm::vec3& with_rotation);

		[[nodiscard]] glm::vec3 getPosition() const noexcept;
		[[nodiscard]] glm::vec3 getSize() const noexcept;
		[[nodiscard]] glm::vec3 getRotation() const noexcept;

		[[nodiscard]] Rendering::IRenderer* getRenderer()
		{
			return renderer;
		}
		[[nodiscard]] Rendering::IMeshBuilder* getMeshBuilder()
		{
			return mesh_builder;
		}

		void addChild(SceneObject* with_child);
		void removeChildren();
		void setParent(SceneObject* with_parent);

		void setParentTransform(const Rendering::Transform& with_parent_transform);

	private:
		// Initialize parent transform so that the object renders without parent properly
		Rendering::Transform parent_transform = {glm::vec3(0), glm::vec3(1, 1, 1), glm::vec3(0)};
		Rendering::Transform transform		  = {};

		SceneObject*			  parent = nullptr;
		std::vector<SceneObject*> children;

		Rendering::IRenderer*	 renderer	  = nullptr;
		Rendering::IMeshBuilder* mesh_builder = nullptr;

		void updateRenderer();
	};
} // namespace Engine
