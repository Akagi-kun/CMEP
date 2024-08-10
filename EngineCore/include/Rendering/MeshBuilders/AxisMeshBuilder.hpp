#pragma once

#include "IMeshBuilder.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering
{
	class AxisMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;

		using IMeshBuilder::SupplyData;

		void Build() override;

		[[nodiscard]] VkPrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		}

		static constexpr bool supports_2d = false;
		static constexpr bool supports_3d = true;
	};
} // namespace Engine::Rendering
