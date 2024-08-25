#pragma once

#include "Rendering/Transform.hpp"

#include "IMeshBuilder.hpp"

namespace Engine::Rendering
{
	class TextMeshBuilder final : public IMeshBuilder
	{
	public:
		using IMeshBuilder::IMeshBuilder;

		void SupplyData(const RendererSupplyData& data) override;

		void Build() override;

		[[nodiscard]] vk::PrimitiveTopology GetSupportedTopology() const noexcept override
		{
			return vk::PrimitiveTopology::eTriangleList;
		}

		static constexpr bool supports_2d = true;
		static constexpr bool supports_3d = true;

	private:
		ScreenSize screen_size{};

		std::string text;

		std::shared_ptr<Rendering::Font> font = nullptr;
	};
} // namespace Engine::Rendering
