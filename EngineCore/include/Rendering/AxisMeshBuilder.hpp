#pragma once

#include "IMeshBuilder.hpp"
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
	};
} // namespace Engine::Rendering
