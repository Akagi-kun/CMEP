#pragma once

#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Transform.hpp"

#include "InternalEngineObject.hpp"

namespace Engine
{
	class Object final : public InternalEngineObject
	{
	public:
		Object(
			Engine* with_engine,
			Rendering::IRenderer* with_renderer,
			Rendering::IMeshBuilder* with_mesh_builder
		);
		~Object() noexcept;

		void ScreenSizeInform(Rendering::ScreenSize with_screen_size);

		// TODO: relative functions?
		void SetPosition(glm::vec3 with_pos);
		void SetSize(glm::vec3 with_size);
		void SetRotation(glm::vec3 with_rotation);

		[[nodiscard]] Rendering::IRenderer* GetRenderer()
		{
			return renderer;
		}
		[[nodiscard]] Rendering::IMeshBuilder* GetMeshBuilder()
		{
			return mesh_builder;
		}

		[[nodiscard]] glm::vec3 GetPosition() const noexcept;
		[[nodiscard]] glm::vec3 GetSize() const noexcept;
		[[nodiscard]] glm::vec3 GetRotation() const noexcept;

		void SetParentTransform(Rendering::Transform with_parent_transform);

		void AddChild(Object* with_child);
		void RemoveChildren();
		void SetParent(Object* with_parent);

	private:
		Rendering::Transform transform;
		// Initialize parent transform so that the object renders without parent properly
		Rendering::Transform parent_transform = {glm::vec3(0), glm::vec3(1, 1, 1), glm::vec3(0)};
		Rendering::ScreenSize screen;

		Object* parent;

		std::vector<Object*> children;

		Rendering::IRenderer* renderer		  = nullptr;
		Rendering::IMeshBuilder* mesh_builder = nullptr;

		void UpdateRenderer();
	};
} // namespace Engine
