#pragma once

#include "IRenderer.hpp"

#include <memory>

namespace Engine::Rendering
{
	class Texture;

	class Renderer3D final : public IRenderer
	{
	private:
		std::shared_ptr<Rendering::Texture> texture = nullptr;

	public:
		Renderer3D(Engine* engine, IMeshBuilder* with_builder, const char* with_pipeline_program);
		~Renderer3D() override;

		void SupplyData(const RendererSupplyData& data) override;

		void UpdateMesh() override;
		void UpdateMatrices() override;
	};
} // namespace Engine::Rendering
