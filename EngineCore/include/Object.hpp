#pragma once

#include "Rendering/IRenderer.hpp"
#include "Rendering/Transform.hpp"

#include "IModule.hpp"
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

		IModule* renderer	  = nullptr;
		IModule* mesh_builder = nullptr;

	public:
		Object() noexcept;
		~Object() noexcept;

		// Overrides InternalEngineObject::UpdateHeldLogger
		void UpdateHeldLogger(std::shared_ptr<Logging::Logger>& new_logger);

		void ScreenSizeInform(unsigned int with_screenx, unsigned int with_screeny) noexcept;

		// Rename to Set...
		void Translate(glm::vec3 with_pos) noexcept;
		void Scale(glm::vec3 with_size) noexcept;
		void Rotate(glm::vec3 with_rotation) noexcept;

		void ModuleBroadcast(ModuleMessageTarget for_modules, const ModuleMessage& data);

		void UpdateRenderer() noexcept;

		[[nodiscard]]
		IModule* AssignRenderer(Rendering::IRenderer* with_renderer);
		Rendering::IRenderer* GetRenderer() noexcept;

		// TODO: Remove
		int Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		// TODO: Rename to Get...
		[[nodiscard]] glm::vec3 Position() const noexcept;
		[[nodiscard]] glm::vec3 Size() const noexcept;
		[[nodiscard]] glm::vec3 Rotation() const noexcept;

		void SetParentPositionRotationSize(Rendering::Transform with_parent_transform);

		void AddChild(Object* with_child);
		void RemoveChildren();
		void SetParent(Object* with_parent);
	};
} // namespace Engine
