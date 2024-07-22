#pragma once

#include "IMeshBuilder.hpp"
#include "vulkan/vulkan_core.h"
namespace Engine::Rendering
{
	class AxisMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;

		void SupplyData(const RendererSupplyData& data) override
		{
			(void)(data);
		}

		void Build() override;

		[[nodiscard]] VkPrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		}
	};
} // namespace Engine::Rendering
