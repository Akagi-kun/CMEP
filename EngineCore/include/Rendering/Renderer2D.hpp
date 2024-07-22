#pragma once

#include "IRenderer.hpp"

#include <memory>
#include <string>

namespace Engine::Rendering
{
	class Texture;
	class Font;

	class Renderer2D final : public IRenderer
	{
	private:
		std::string text;

		std::shared_ptr<Rendering::Texture> texture = nullptr;

		glm::mat4 mat_mvp{};

	public:
		Renderer2D(
			Engine* engine,
			IMeshBuilder* with_builder,
			const char* with_pipeline_program,
			VkPrimitiveTopology with_primitives
		);
		~Renderer2D() override;

		void SupplyData(const RendererSupplyData& data) override;

		void UpdateMesh() override;

		void Render(VkCommandBuffer command_buffer, uint32_t current_frame) override;

		[[nodiscard]] bool GetIsUI() const override
		{
			return true;
		}
	};
} // namespace Engine::Rendering
