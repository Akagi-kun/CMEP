#pragma once

#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace Engine::Rendering
{
	class Font;
	class Texture;

	enum class RendererSupplyDataType : uint8_t
	{
		MIN_ENUM = 0x00,

		TEXTURE = 1,
		SCRIPT	= 2,
		FONT	= 3,

		TEXT = 32,

		MAX_ENUM = 0xFF
	};

	struct RendererSupplyData
	{
		using payload_type = std::variant<std::weak_ptr<void>, std::string>;

		RendererSupplyDataType type = RendererSupplyDataType::MIN_ENUM;
		payload_type payload;

		RendererSupplyData(RendererSupplyDataType with_type, payload_type data)
		{
			this->type	  = with_type;
			this->payload = std::move(data);
		}
	};
} // namespace Engine::Rendering
