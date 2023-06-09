#pragma once

#include <memory>

#include "glm/gtc/matrix_transform.hpp"
#include "IRenderer.hpp"
#include "Mesh.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	/// <summary>
	/// Implementation of <seealso cref="IRenderer"/> for custom mesh renderables.
	/// </summary>
	/// <inheritdoc cref="IRenderer"/>
	class __declspec(dllexport) MeshRenderer final : public IRenderer
	{
	private:
		/// <summary>
		/// GL Vertex Array Object
		/// </summary>
		unsigned int vao = 0;
		/// <summary>
		/// GL Vertex Buffer Object
		/// </summary>
		unsigned int vbo = 0;
		/// <summary>
		/// GL Material Buffer Object
		/// </summary>
		unsigned int mbo = 0;
		/// <summary>
		/// GL Tan/Bitan Buffer Object
		/// </summary>
		unsigned int tbbo = 0;

		glm::mat4 matM;
		glm::mat4 matV;
		glm::mat4 matMV;
		glm::mat3 matMV3x3;
		glm::mat4 matMVP;

		/// <summary>
		/// Currently used shader
		/// </summary>
		std::unique_ptr<Shader> program;
		std::unique_ptr<const Rendering::Texture> texture;

		bool has_updated_meshdata = false;

		std::unique_ptr<Mesh> mesh;
	public:
		MeshRenderer();
		~MeshRenderer();

		void AssignMesh(Mesh& new_mesh);

		void UpdateTexture(const Rendering::Texture* texture) noexcept;
		void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;
		void UpdateMesh() noexcept override;

		void Render() override;
	};
}