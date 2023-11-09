#pragma once

#include "glm/vec3.hpp"

#include "VulkanRenderingEngine.hpp"
#include "PlatformSemantics.hpp"

namespace Engine
{
	class Object;

	namespace Rendering
	{
		class Shader;

		/// <summary>
		/// Interface describing Renderer API for renderables.
		/// </summary>
		class CMEP_EXPORT IRenderer
		{
		protected:
			glm::vec3 _pos = glm::vec3();
			glm::vec3 _size = glm::vec3();
			glm::vec3 _rotation = glm::vec3();
			
			glm::vec3 _parent_pos = glm::vec3();
			glm::vec3 _parent_size = glm::vec3();
			glm::vec3 _parent_rotation = glm::vec3();

			uint_fast16_t _screenx = 0, _screeny = 0;

			bool has_updated_mesh = false;

		public:
			IRenderer() {};
			virtual ~IRenderer() {};

			virtual void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny, glm::vec3 parent_position, glm::vec3 parent_rotation, glm::vec3 parent_size) = 0;
			
			virtual void UpdateMesh() = 0;
			
			virtual void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;
		};
	}
}