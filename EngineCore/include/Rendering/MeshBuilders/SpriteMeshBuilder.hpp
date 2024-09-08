#pragma once

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class SpriteMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;
		using IMeshBuilder::supplyData;

		void build() override;

		[[nodiscard]] vk::PrimitiveTopology getSupportedTopology() const noexcept override
		{
			return vk::PrimitiveTopology::eTriangleList;
		}

		static constexpr bool supports_2d = true;
		static constexpr bool supports_3d = true;
	};
} // namespace Engine::Rendering
