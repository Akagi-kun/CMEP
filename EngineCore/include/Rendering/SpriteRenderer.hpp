#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "IRenderer.hpp"
#include "PlatformSemantics.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class SpriteRenderer final : public IRenderer
	{
	private:
		VulkanBuffer* vbo = nullptr;

		glm::mat4 mat_mvp{};

		VulkanPipeline* pipeline = nullptr;

	public:
		// TODO: Make private after fixes in Scene
		std::shared_ptr<const Rendering::Texture> texture;

		SpriteRenderer(Engine* engine);
		~SpriteRenderer() override;

		void SupplyData(RendererSupplyData data) override;

		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override
		{
			return true;
		}
	};
} // namespace Engine::Rendering
