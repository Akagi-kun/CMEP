#pragma once

#include "Rendering/SupplyData.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class GeneratorMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;
		using IMeshBuilder::SupplyData;

		void SupplyData(const MeshBuilderSupplyData& data) override;

		void Build() override;

		[[nodiscard]] vk::PrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return vk::PrimitiveTopology::eTriangleList;
		}

		static constexpr bool supports_2d = true;
		static constexpr bool supports_3d = true;

	private:
		GeneratorData script_data;
	};
} // namespace Engine::Rendering
