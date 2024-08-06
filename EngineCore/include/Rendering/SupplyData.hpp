#pragma once

#include <cassert>
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

		bool operator==(const RendererSupplyData& other) const
		{
			if (type != other.type)
			{
				return false;
			}

			// type == other.type is true here
			switch (type)
			{
				case RendererSupplyDataType::FONT:
				{
					const auto& payload_ref = std::get<std::weak_ptr<void>>(payload);
					assert(!payload_ref.expired() && "Cannot lock expired payload!");
					auto font_cast = std::static_pointer_cast<Font>(payload_ref.lock());

					const auto& other_payload_ref = std::get<std::weak_ptr<void>>(payload);
					assert(!other_payload_ref.expired() && "Cannot lock expired payload!");
					auto other_font_cast = std::static_pointer_cast<Font>(other_payload_ref.lock());

					return font_cast == other_font_cast;
				}
				case RendererSupplyDataType::TEXTURE:
				{
					const auto& payload_ref = std::get<std::weak_ptr<void>>(payload);
					assert(!payload_ref.expired() && "Cannot lock expired payload!");
					auto texture_cast = std::static_pointer_cast<Texture>(payload_ref.lock());

					const auto& other_payload_ref = std::get<std::weak_ptr<void>>(other.payload);
					assert(!other_payload_ref.expired() && "Cannot lock expired payload!");
					auto other_texture_cast = std::static_pointer_cast<Texture>(other_payload_ref.lock());

					return texture_cast == other_texture_cast;
				}
				default:
				{
					break;
				}
			}

			return false;
		}
	};
} // namespace Engine::Rendering
