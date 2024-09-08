#pragma once

#include "Rendering/Transform.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class TextMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;

		void supplyData(const MeshBuilderSupplyData& data) override;

		void build() override;

		[[nodiscard]] vk::PrimitiveTopology getSupportedTopology() const noexcept override
		{
			return vk::PrimitiveTopology::eTriangleList;
		}

		static constexpr bool supports_2d = true;
		static constexpr bool supports_3d = true;

	private:
		ScreenSize screen_size{};

		std::string text;

		std::shared_ptr<const Rendering::Font> font = nullptr;
	};
} // namespace Engine::Rendering
