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

		std::shared_ptr<Rendering::Texture> texture;
		std::shared_ptr<Mesh> mesh;

		bool has_updated_meshdata = false;

		void AssignMesh(std::shared_ptr<Mesh> new_mesh);
		void UpdateTexture(std::shared_ptr<Rendering::Texture> texture);

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
		
		void SupplyData(RendererSupplyData data) override;
		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

		bool GetIsUI() const override { return false; }
	};
} // namespace Engine::Rendering