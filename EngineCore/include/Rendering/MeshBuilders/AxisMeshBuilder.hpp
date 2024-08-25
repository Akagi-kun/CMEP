#pragma once

#include "IMeshBuilder.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering
{
	class AxisMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;

		using IMeshBuilder::SupplyData;

		void Build() override;

		[[nodiscard]] vk::PrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return vk::PrimitiveTopology::eLineList;
		}

		static constexpr bool supports_2d = false;
		static constexpr bool supports_3d = true;
	};
} // namespace Engine::Rendering
