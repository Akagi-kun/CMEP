#pragma once

#include "IRenderer.hpp"

namespace Engine::Rendering
{
	class Texture;

	class Renderer2D final : public IRenderer
	{
	public:
		Renderer2D(Engine* engine, IMeshBuilder* with_builder, std::string_view with_pipeline_program);
		~Renderer2D() override;

		void SupplyData(const RendererSupplyData& data) override;

		void UpdateMatrices() override;
	};
} // namespace Engine::Rendering
