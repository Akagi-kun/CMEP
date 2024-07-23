#pragma once

#include "IRenderer.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;

	class Renderer2D final : public IRenderer
	{
	private:
		std::shared_ptr<Rendering::Texture> texture = nullptr;

	public:
		Renderer2D(Engine* engine, IMeshBuilder* with_builder, const char* with_pipeline_program);
		~Renderer2D() override;

		void SupplyData(const RendererSupplyData& data) override;

		void UpdateMesh() override;
	};
} // namespace Engine::Rendering
