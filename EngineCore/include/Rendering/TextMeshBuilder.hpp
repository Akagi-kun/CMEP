#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/Wrappers/VSampledImage.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class TextMeshBuilder final : public IMeshBuilder
	{
		ScreenSize screen_size{};

		std::string text;

		Vulkan::VSampledImage* texture_image = nullptr;

		std::shared_ptr<Rendering::Font> font = nullptr;

	public:
		using IMeshBuilder::IMeshBuilder;

		void SupplyData(const RendererSupplyData& data) override;

		void Build() override;

		[[nodiscard]] VkPrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	};
} // namespace Engine::Rendering
