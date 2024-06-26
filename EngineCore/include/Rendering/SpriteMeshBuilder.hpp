#pragma once

#include "IMeshBuilder.hpp"
namespace Engine::Rendering
{
	class SpriteMeshBuilder final : public IMeshBuilder
	{
	public:
		SpriteMeshBuilder(Engine* engine, VulkanRenderingEngine* with_renderer) : IMeshBuilder(engine, with_renderer)
		{
		}

		void Build() override;

		// MeshBuildContext& GetContext() override;
	};
} // namespace Engine::Rendering
