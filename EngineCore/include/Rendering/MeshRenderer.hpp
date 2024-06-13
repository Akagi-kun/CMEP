#pragma once
#include "Assets/Mesh.hpp"

#include "IRenderer.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	/// <summary>
	/// Implementation of <seealso cref="IRenderer"/> for custom mesh renderables.
	/// </summary>
	/// <inheritdoc cref="IRenderer"/>
	class MeshRenderer final : public IRenderer
	{
	private:
		size_t vbo_vert_count = 0;

		VulkanPipeline* pipeline = nullptr;
		VulkanBuffer* vbo = nullptr;

		glm::mat4 mat_m{};
		glm::mat4 mat_v{};
		glm::mat4 mat_mv{};
		glm::mat3 mat_mv_3x3{};
		glm::mat4 mat_mvp{};

		std::shared_ptr<Rendering::Texture> texture;
		std::shared_ptr<Mesh> mesh;

		bool has_updated_meshdata = false;

	public:
		MeshRenderer(Engine* engine);
		~MeshRenderer() override;

		void SupplyData(RendererSupplyData data) override;
		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override
		{
			return false;
		}
	};
} // namespace Engine::Rendering
