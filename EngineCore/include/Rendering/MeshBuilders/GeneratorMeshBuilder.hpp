#pragma once

#include "Rendering/SupplyData.hpp"

#include "Scripting/ILuaScript.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class GeneratorMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;
		using IMeshBuilder::SupplyData;

		void SupplyData(const RendererSupplyData& data) override;

		void Build() override;

		[[nodiscard]] vk::PrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return vk::PrimitiveTopology::eTriangleList; // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		static constexpr bool supports_2d = true;
		static constexpr bool supports_3d = true;

	private:
		std::shared_ptr<Scripting::ILuaScript> generator_script;

		GeneratorSupplierData generator_supplier;
	};
} // namespace Engine::Rendering
