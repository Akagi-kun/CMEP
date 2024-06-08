#pragma once

#include <memory>

#include "IRenderer.hpp"
#include "Mesh.hpp"
#include "PlatformSemantics.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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

		glm::mat4 matM{};
		glm::mat4 matV{};
		glm::mat4 matMV{};
		glm::mat3 matMV3x3{};
		glm::mat4 matMVP{};

		std::unique_ptr<const Rendering::Texture> texture;

		bool has_updated_meshdata = false;

		std::shared_ptr<Mesh> mesh;

	public:
		MeshRenderer(Engine* engine);
		~MeshRenderer();

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

		void AssignMesh(std::shared_ptr<Mesh> new_mesh);
		void UpdateTexture(const Rendering::Texture* texture);
		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override { return false; }
	};
} // namespace Engine::Rendering