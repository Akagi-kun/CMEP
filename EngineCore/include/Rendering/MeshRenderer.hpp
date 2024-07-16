#pragma once
#include "Assets/Mesh.hpp"

#include "IRenderer.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class MeshRenderer final : public IRenderer
	{
	private:
		glm::mat4 mat_mvp{};

		// std::shared_ptr<Rendering::Texture> texture;
		std::shared_ptr<Mesh> mesh;

		bool has_updated_meshdata = false;

	public:
		MeshRenderer(Engine* engine, const char* with_pipeline_program);
		~MeshRenderer() override;

		void SupplyData(const RendererSupplyData& data) override;
		void UpdateMesh() override;

		void Render(VkCommandBuffer command_buffer, uint32_t current_frame) override;

		[[nodiscard]] bool GetIsUI() const override
		{
			return false;
		}
	};
} // namespace Engine::Rendering
