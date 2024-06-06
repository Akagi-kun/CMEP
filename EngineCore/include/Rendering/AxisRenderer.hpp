#pragma once

#include <memory>

#include "IRenderer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	/// <summary>
	/// Implementation of <seealso cref="IRenderer"/> for an axis.
	/// </summary>
	/// <inheritdoc cref="IRenderer"/>
	class AxisRenderer final : public IRenderer
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

		VulkanPipeline* pipeline = nullptr;
		VulkanBuffer* vbo = nullptr;

		glm::mat4 matMVP{};

	public:
		AxisRenderer(Engine* engine);
		~AxisRenderer();

		void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny, glm::vec3 parent_position, glm::vec3 parent_rotation, glm::vec3 parent_size) override;
		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override { return false; }
	};
}