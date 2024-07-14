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

		std::shared_ptr<const Rendering::Texture> texture;

	public:
		SpriteRenderer(Engine* engine, IMeshBuilder* with_builder);
		~SpriteRenderer() override;

		void SupplyData(const RendererSupplyData& data) override;

		void UpdateMesh() override;

		void Render(VkCommandBuffer command_buffer, uint32_t current_frame) override;

		[[nodiscard]] bool GetIsUI() const override
		{
			return true;
		}
	};
} // namespace Engine::Rendering
