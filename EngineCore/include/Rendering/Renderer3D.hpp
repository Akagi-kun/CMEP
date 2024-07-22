#pragma once

#include "IRenderer.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;
	class Shader;

	class Renderer3D final : public IRenderer
	{
	private:
		glm::mat4 mat_mvp{};

		std::shared_ptr<Rendering::Texture> texture = nullptr;

		bool has_updated_meshdata = false;

	public:
		Renderer3D(Engine* engine, IMeshBuilder* with_builder, const char* with_pipeline_program);
		~Renderer3D() override;

		void SupplyData(const RendererSupplyData& data) override;
		void UpdateMesh() override;

		void Render(VkCommandBuffer command_buffer, uint32_t current_frame) override;

		[[nodiscard]] bool GetIsUI() const override
		{
			return false;
		}
	};
} // namespace Engine::Rendering
