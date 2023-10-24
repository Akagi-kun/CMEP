#pragma once

#include <memory>
#include <string>

#include "IRenderer.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;
	class Font;

	/// <summary>
	/// Implementation of <seealso cref="IRenderer"/> for text renderables.
	/// </summary>
	/// <inheritdoc cref="IRenderer"/>
	class __declspec(dllexport) TextRenderer final : public IRenderer
	{
	private:
		/// <summary>
		/// GL Vertex Array Object
		/// </summary>
		//unsigned int vao = 0;
		/// <summary>
		/// GL Vertex Buffer Object
		/// </summary>
		//unsigned int vbo = 0;
		/// <summary>
		/// Count of vertices in Vertex Buffer Object
		/// </summary>
		size_t vbo_vert_count = 0;
		/// <summary>
		/// Text to be rendered
		/// </summary>
		std::string text = "";

		VulkanPipeline* pipeline = nullptr;
		VulkanTextureImage* textureImage = nullptr;
		VulkanBuffer* vbo = nullptr;

		/// <summary>
		/// Currently used shader
		/// </summary>
		std::unique_ptr<Rendering::Shader> program;
		/// <summary>
		/// Currently used font
		/// </summary>
		std::unique_ptr<Rendering::Font> font;

	public:
		TextRenderer();
		~TextRenderer();

		void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;

		/// <summary>
		/// Update font used by renderer. See <see cref="font"/>.
		/// </summary>
		/// <param name="font">New font.</param>
		int UpdateFont(Rendering::Font* const font) noexcept;
		
		/// <summary>
		/// Update rendered text.
		/// </summary>
		/// <param name="text">New text.</param>
		int UpdateText(const std::string text) noexcept;
		
		void UpdateMesh() override;
		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
	};
}