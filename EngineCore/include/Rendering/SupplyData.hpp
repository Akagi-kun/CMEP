#pragma once

#include "Assets/Font.hpp"

#include "Scripting/ILuaScript.hpp"

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

	struct GeneratorData
	{
		Scripting::ScriptFunctionRef generator;
		Scripting::ScriptFunctionRef supplier;
	};

	struct RendererSupplyData
	{
		enum class Type : uint8_t
		{
			// weak_ptr<void>
			FONT	= 4,
			TEXTURE = 20,
		};

		using payload_t = std::variant<std::weak_ptr<void>>;

		Type	  type;
		payload_t payload;

		RendererSupplyData(Type with_type, payload_t data)
		{
			type	= with_type;
			payload = std::move(data);
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
				case Type::FONT:
				{
					return PayloadCompareOp<std::weak_ptr<void>, Font>(payload, other.payload);
				}
				case Type::TEXTURE:
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

		template <typename value_t, typename cast_t>
		[[nodiscard]] bool PayloadCompareOp(payload_t left, payload_t right) const
			requires(std::is_same_v<value_t, std::weak_ptr<void>>)
		{
			const auto& left_payload_ref = std::get<value_t>(left);
			const auto	left_cast = std::static_pointer_cast<cast_t>(left_payload_ref.lock());

			const auto& right_payload_ref = std::get<value_t>(right);
			const auto	right_cast = std::static_pointer_cast<cast_t>(right_payload_ref.lock());

			return left_cast == right_cast;
		}
	};

	struct MeshBuilderSupplyData
	{
		enum class Type : uint8_t
		{
			// Only to be used internally
			// needed by Renderer to inform MeshBuilder of font information
			FONT = 4,

			// GeneratorData
			GENERATOR = 8,

			// std::string
			TEXT = 32,
		};

		using payload_t = std::variant<std::string, std::weak_ptr<Font>, GeneratorData>;

		Type	  type;
		payload_t payload;

		MeshBuilderSupplyData(Type with_type, payload_t data)
		{
			type	= with_type;
			payload = std::move(data);
		}
	};
} // namespace Engine::Rendering
