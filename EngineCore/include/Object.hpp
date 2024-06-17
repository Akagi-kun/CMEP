#pragma once

#include "Rendering/IRenderer.hpp"
#include "Rendering/Transform.hpp"

#include "InternalEngineObject.hpp"
// #include <functional>

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

		// std::function<void(Object*)> on_click = nullptr;

		Rendering::IRenderer* renderer = nullptr;

	public:
		Object() noexcept;
		~Object() noexcept;

		// Overrides InternalEngineObject::UpdateHeldLogger
		void UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger);

		void ScreenSizeInform(unsigned int with_screenx, unsigned int with_screeny) noexcept;

		void Translate(glm::vec3 with_pos) noexcept;
		void Scale(glm::vec3 with_size) noexcept;
		void Rotate(glm::vec3 with_rotation) noexcept;

		void UpdateRenderer() noexcept;

		[[nodiscard]]
		Rendering::IRenderer* AssignRenderer(Rendering::IRenderer* with_renderer);
		Rendering::IRenderer* GetRenderer() noexcept;

		int Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		[[nodiscard]] glm::vec3 Position() const noexcept;
		[[nodiscard]] glm::vec3 Size() const noexcept;
		[[nodiscard]] glm::vec3 Rotation() const noexcept;

		void SetParentPositionRotationSize(Rendering::Transform with_parent_transform);

		void AddChild(Object* with_child);
		void RemoveChildren();
		void SetParent(Object* with_parent);
	};
} // namespace Engine
