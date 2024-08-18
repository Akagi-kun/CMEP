#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class TextMeshBuilder final : public IMeshBuilder
	{
		ScreenSize screen_size{};

		std::string text;

		Vulkan::SampledImage* texture_image = nullptr;

		std::shared_ptr<Rendering::Font> font = nullptr;

	public:
		using IMeshBuilder::IMeshBuilder;

		void SupplyData(const RendererSupplyData& data) override;

		void Build() override;

		[[nodiscard]] VkPrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		static constexpr bool supports_2d = true;
		static constexpr bool supports_3d = true;
	};
} // namespace Engine::Rendering
