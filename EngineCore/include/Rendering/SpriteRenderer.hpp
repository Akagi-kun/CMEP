#pragma once

#include "Rendering/IMeshBuilder.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "IRenderer.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class SpriteRenderer final : public IRenderer
	{
	private:
		glm::mat4 mat_mvp{};

	public:
		// TODO: Make private after fixes in Scene
		std::shared_ptr<const Rendering::Texture> texture;

		SpriteRenderer(Engine* engine, IMeshBuilder* with_builder);
		~SpriteRenderer() override;

		void SupplyData(const RendererSupplyData& data) override;

		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		[[nodiscard]] bool GetIsUI() const override
		{
			return true;
		}
	};
} // namespace Engine::Rendering
