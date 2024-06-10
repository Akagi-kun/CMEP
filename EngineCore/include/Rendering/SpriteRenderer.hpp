#pragma once

#include <memory>

#include "IRenderer.hpp"
#include "PlatformSemantics.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class SpriteRenderer final : public IRenderer
	{
	private:
		VulkanBuffer* vbo = nullptr;

		glm::mat4 matMVP{};

		VulkanPipeline* pipeline = nullptr;

	public:
		// TODO: Make private after fixes in Scene
		std::shared_ptr<const Rendering::Texture> texture;

		SpriteRenderer(Engine* engine);
		~SpriteRenderer();

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

		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override
		{
			return true;
		}
	};
} // namespace Engine::Rendering