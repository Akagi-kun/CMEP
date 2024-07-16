#pragma once

#include "Rendering/IRenderer.hpp"
#include "Rendering/Transform.hpp"

#include "InternalEngineObject.hpp"

namespace Engine
{
	class Object final : public InternalEngineObject
	{
	private:
		Rendering::Transform transform;
		Rendering::Transform parent_transform;
		Rendering::ScreenSize screen;

		Object* parent;

		std::vector<Object*> children;

		Rendering::IRenderer* renderer = nullptr;

		void UpdateRenderer() noexcept;

	public:
		using InternalEngineObject::InternalEngineObject;
		~Object() noexcept;

		void ScreenSizeInform(unsigned int with_screenx, unsigned int with_screeny) noexcept;

		// TODO: relative functions?
		void SetPosition(glm::vec3 with_pos) noexcept;
		void SetSize(glm::vec3 with_size) noexcept;
		void SetRotation(glm::vec3 with_rotation) noexcept;

		void SetRenderer(Rendering::IRenderer* with_renderer);
		Rendering::IRenderer* GetRenderer();

		[[nodiscard]] glm::vec3 GetPosition() const noexcept;
		[[nodiscard]] glm::vec3 GetSize() const noexcept;
		[[nodiscard]] glm::vec3 GetRotation() const noexcept;

		void SetParentPositionRotationSize(Rendering::Transform with_parent_transform);

		void AddChild(Object* with_child);
		void RemoveChildren();
		void SetParent(Object* with_parent);
	};
} // namespace Engine
