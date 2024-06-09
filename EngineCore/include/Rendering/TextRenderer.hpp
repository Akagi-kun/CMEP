#pragma once

#include <memory>
#include <string>

#include "IRenderer.hpp"
#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Font;

	class TextRenderer final : public IRenderer
	{
	private:
		size_t vbo_vert_count = 0;

		std::string text = "";

		VulkanPipeline* pipeline = nullptr;
		VulkanBuffer* vbo = nullptr;
		VulkanTextureImage* textureImage = nullptr;

		std::shared_ptr<Rendering::Font> font = nullptr;

		glm::mat4 matMVP{};

		int UpdateFont(std::shared_ptr<Rendering::Font> font);
		
	public:
		TextRenderer(Engine* engine);
		~TextRenderer();

		void Update(
			glm::vec3 pos,
			glm::vec3 size,
			glm::vec3 rotation,
			uint_fast16_t screenx,
			uint_fast16_t screeny,
			glm::vec3 parent_position,
			glm::vec3 parent_rotation,
			glm::vec3 parent_size
		) override;

		void SupplyData(RendererSupplyData data) override;

		int UpdateText(const std::string text);
		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override { return true; }
	};
} // namespace Engine::Rendering