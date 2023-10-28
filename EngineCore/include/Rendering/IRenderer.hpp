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
		/// Interface describing GL Renderer API for renderables.
		/// </summary>
		class CMEP_EXPORT IRenderer
		{
		protected:
			/// <summary>
			/// Renderable's position.
			/// </summary>
			glm::vec3 _pos = glm::vec3();
			/// <summary>
			/// Renderable's size.
			/// </summary>
			glm::vec3 _size = glm::vec3();
			/// <summary>
			/// Renderable's rotation.
			/// </summary>
			glm::vec3 _rotation = glm::vec3();
			
			/// <summary>
			/// Screen size as reported by <seealso cref="Object"/>.
			/// </summary>
			uint_fast16_t _screenx = 0, _screeny = 0;

			/// <summary>
			/// Used by <seealso cref="UpdateMesh"/> to optimize Updates to when necessary.
			/// </summary>
			bool has_updated_mesh = false;

		public:
			IRenderer() {};

			/// <summary>
			/// Updates data for renderer.
			/// </summary>
			/// <param name="pos">Position of renderable.</param>
			/// <param name="size">Size of renderable.</param>
			/// <param name="screenx">X size of screen.</param>
			/// <param name="screeny">Y size of screen.</param>
			virtual void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept = 0;
			
			/// <summary>
			/// Updates mesh of renderable.
			/// </summary>
			virtual void UpdateMesh() = 0;
			
			/// <summary>
			/// Render the renderable represented by this <seealso cref="Renderer"/>.
			/// </summary>
			virtual void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;
		};
	}
}