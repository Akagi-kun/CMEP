#pragma once

#include <memory>

#include "IRenderer.hpp"
#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	/// <summary>
	/// Implementation of <seealso cref="IRenderer"/> for 2D sprite renderables.
	/// </summary>
	/// <inheritdoc cref="IRenderer"/>
	class CMEP_EXPORT SpriteRenderer final : public IRenderer
	{
	private:
		/// <summary>
		/// GL Vertex Array Object
		/// </summary>
		unsigned int vao = 0;
		/// <summary>
		/// GL Vertex Buffer Object
		/// </summary>
		unsigned int vbo = 0;

		std::unique_ptr<Rendering::Shader> program;
		std::unique_ptr<const Rendering::Texture> texture;

	public:
		SpriteRenderer();
		~SpriteRenderer();

		void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;
		void UpdateTexture(const Rendering::Texture* texture) noexcept;
		void UpdateMesh() noexcept override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
	};
}