#pragma once

#include "Rendering/IRenderer.hpp"

#include "InternalEngineObject.hpp"
// #include <functional>

namespace Engine
{
	class Object final : public InternalEngineObject
	{
	private:
		glm::vec3 pos = glm::vec3();
		glm::vec3 size = glm::vec3();
		glm::vec3 rotation = glm::vec3();

		glm::vec3 parent_pos = glm::vec3();
		glm::vec3 parent_size = glm::vec3();
		glm::vec3 parent_rotation = glm::vec3();

		Object* parent = nullptr;

		std::vector<Object*> children;

		unsigned int screenx = 0, screeny = 0;

		// std::function<void(Object*)> on_click = nullptr;

	public:
		Rendering::IRenderer* renderer = nullptr;
		// std::string renderer_type;

		Object() noexcept;
		~Object() noexcept;

		void ScreenSizeInform(unsigned int new_screenx, unsigned int new_screeny) noexcept;

		void Translate(const glm::vec3 new_pos) noexcept;

		void Scale(const glm::vec3 new_size) noexcept;

		void Rotate(const glm::vec3 new_rotation) noexcept;

		void UpdateRenderer() noexcept;

		int Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		glm::vec3 Position() const noexcept;
		glm::vec3 Size() const noexcept;
		glm::vec3 Rotation() const noexcept;

		void SetParentPositionRotationSize(glm::vec3 new_position, glm::vec3 new_rotation, glm::vec3 new_size);

		void AddChild(Object* object);
		void RemoveChildren();
		void SetParent(Object* object);
	};
} // namespace Engine
