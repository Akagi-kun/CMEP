#pragma once

#include <functional>

#include "InternalEngineObject.hpp"
#include "Logging/Logging.hpp"
#include "PlatformSemantics.hpp"
#include "Rendering/IRenderer.hpp"

namespace Engine
{
	class Object : public InternalEngineObject
	{
	protected:
		/// <summary>
		/// Position of object in worldspace.
		/// </summary>
		glm::vec3 pos = glm::vec3();

		/// <summary>
		/// Size of object.
		/// </summary>
		glm::vec3 size = glm::vec3();

		/// <summary>
		/// Rotation of object.
		/// </summary>
		glm::vec3 rotation = glm::vec3();

		/// <summary>
		/// Parent pos size and rot
		/// </summary>
		glm::vec3 parent_pos = glm::vec3();
		glm::vec3 parent_size = glm::vec3();
		glm::vec3 parent_rotation = glm::vec3();

		Object* parent = nullptr;

		std::vector<Object*> children;

		unsigned int screenx = 0, screeny = 0;

		std::function<void(Object*)> on_click = nullptr;

	public:
		Rendering::IRenderer* renderer = nullptr;
		std::string renderer_type = "";

		Object() noexcept;
		~Object() noexcept;

		void ScreenSizeInform(unsigned int screenx, unsigned int screeny) noexcept;

		virtual void Translate(const glm::vec3 pos) noexcept;

		virtual void Scale(const glm::vec3 size) noexcept;

		virtual void Rotate(const glm::vec3 rotation) noexcept;

		virtual void UpdateRenderer() noexcept;

		virtual int Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		glm::vec3 Position() const noexcept;
		glm::vec3 Size() const noexcept;
		glm::vec3 Rotation() const noexcept;

		void SetParentPositionRotationSize(glm::vec3 position, glm::vec3 rotation, glm::vec3 size);

		void AddChild(Object* object);
		void RemoveChildren();
		void SetParent(Object* object);
	};
} // namespace Engine