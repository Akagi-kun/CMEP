#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "IRenderer.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class AxisRenderer final : public IRenderer
	{
	private:
		glm::mat4 mat_mvp{};

	public:
		AxisRenderer(Engine* engine);
		~AxisRenderer() override;

		void SupplyData(const RendererSupplyData& data) override
		{
		}

		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		[[nodiscard]] bool GetIsUI() const override
		{
			return false;
		}
	};
} // namespace Engine::Rendering
