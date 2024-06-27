#pragma once

#include "Rendering/Transform.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class TextMeshBuilder final : public IMeshBuilder
	{
		ScreenSize screen_size{};

		std::string text;

		VulkanTextureImage* texture_image = nullptr;

		std::shared_ptr<Rendering::Font> font = nullptr;

	public:
		TextMeshBuilder(Engine* engine, VulkanRenderingEngine* with_renderer) : IMeshBuilder(engine, with_renderer)
		{
		}

		void SupplyData(const RendererSupplyData& data) override;

		void Build() override;
	};
} // namespace Engine::Rendering
