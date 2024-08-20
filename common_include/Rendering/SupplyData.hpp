#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace Engine::Scripting
{
	class ILuaScript;
}

namespace Engine::Rendering
{
	class Font;
	class Texture;

	struct GeneratorSupplierData
	{
		std::weak_ptr<Scripting::ILuaScript> script;
		std::string name;
	};

	enum class RendererSupplyDataType : uint8_t
	{
		MIN_ENUM = 0x00,

		// weak_ptr<void>
		TEXTURE			 = 1,
		GENERATOR_SCRIPT = 2,
		FONT			 = 3,

		// GeneratorSupplierData
		GENERATOR_SUPPLIER = 16,

		// std::string
		TEXT = 32,

		MAX_ENUM = 0xFF
	};

	struct RendererSupplyData
	{
		using payload_t = std::variant<std::weak_ptr<void>, std::string, GeneratorSupplierData>;

		RendererSupplyDataType type = RendererSupplyDataType::MIN_ENUM;
		payload_t payload;

		RendererSupplyData(RendererSupplyDataType with_type, payload_t data)
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
					return PayloadCompareOp<std::weak_ptr<void>, Font>(payload, other.payload);
				}
				case RendererSupplyDataType::TEXTURE:
				{
					return PayloadCompareOp<std::weak_ptr<void>, Texture>(payload, other.payload);
				}
				default:
				{
					break;
				}
			}

			return false;
		}

		template <
			typename value_t,
			typename cast_t,
			std::enable_if_t<std::is_same_v<value_t, std::weak_ptr<void>>, int>* = nullptr>
		[[nodiscard]] bool PayloadCompareOp(payload_t left, payload_t right) const
		{
			const auto& left_payload_ref = std::get<value_t>(left);
			const auto left_cast		 = std::static_pointer_cast<cast_t>(left_payload_ref.lock());

			const auto& right_payload_ref = std::get<value_t>(right);
			const auto right_cast		  = std::static_pointer_cast<cast_t>(right_payload_ref.lock());

			return left_cast == right_cast;
		}
	};
} // namespace Engine::Rendering
