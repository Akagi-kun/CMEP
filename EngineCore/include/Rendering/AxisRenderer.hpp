#pragma once

#include <memory>

#include "IRenderer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "PlatformSemantics.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class AxisRenderer final : public IRenderer
	{
	private:
		VulkanPipeline* pipeline = nullptr;
		VulkanBuffer* vbo = nullptr;

		glm::mat4 matMVP{};

	public:
		AxisRenderer(Engine* engine);
		~AxisRenderer();

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

		int SupplyData(RendererSupplyData data) override { return 1; };

		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override
		{
			return false;
		}
	};
} // namespace Engine::Rendering