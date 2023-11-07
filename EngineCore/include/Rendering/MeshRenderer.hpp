#pragma once

#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "IRenderer.hpp"
#include "Mesh.hpp"
#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	/// <summary>
	/// Implementation of <seealso cref="IRenderer"/> for custom mesh renderables.
	/// </summary>
	/// <inheritdoc cref="IRenderer"/>
	class CMEP_EXPORT MeshRenderer final : public IRenderer
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
		/// <summary>
		/// GL Material Buffer Object
		/// </summary>
		//unsigned int mbo = 0;
		/// <summary>
		/// GL Tan/Bitan Buffer Object
		/// </summary>
		//unsigned int tbbo = 0;

		size_t vbo_vert_count = 0;

		VulkanPipeline* pipeline = nullptr;
		VulkanBuffer* vbo = nullptr;

		glm::mat4 matM{};
		glm::mat4 matV{};
		glm::mat4 matMV{};
		glm::mat3 matMV3x3{};
		glm::mat4 matMVP{};

		/// <summary>
		/// Currently used shader
		/// </summary>
		std::unique_ptr<const Rendering::Texture> texture;

		bool has_updated_meshdata = false;

		Mesh* mesh = nullptr;
	public:
		MeshRenderer();
		~MeshRenderer();

		void AssignMesh(Mesh* new_mesh);

		void UpdateTexture(const Rendering::Texture* texture);
		void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny, glm::vec3 parent_position, glm::vec3 parent_rotation, glm::vec3 parent_size) override;
		void UpdateMesh() override;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
	};
}