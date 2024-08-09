#pragma once

#include "Rendering/IMeshBuilder.hpp"

#include "Scripting/ILuaScript.hpp"

#include "SupplyData.hpp"

namespace Engine::Rendering
{
	class GeneratorMeshBuilder final : public IMeshBuilder
	{
	private:
		std::shared_ptr<Scripting::ILuaScript> generator_script;

		GeneratorSupplierData generator_supplier;

	public:
		using IMeshBuilder::IMeshBuilder;
		using IMeshBuilder::SupplyData;

		void SupplyData(const RendererSupplyData& data) override;

		void Build() override;

		[[nodiscard]] VkPrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	};
} // namespace Engine::Rendering
