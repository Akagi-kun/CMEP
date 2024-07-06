#pragma once

#include "IMeshBuilder.hpp"
namespace Engine::Rendering
{
	class SpriteMeshBuilder final : public IMeshBuilder
	{
	public:
		SpriteMeshBuilder(Engine* engine, Vulkan::VulkanRenderingEngine* with_renderer)
			: IMeshBuilder(engine, with_renderer)
		{
		}

		void SupplyData(const RendererSupplyData& data) override
		{
			(void)(data);
		}

		void Build() override;
	};
} // namespace Engine::Rendering
