#pragma once

#include "Exception.hpp"

#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace Engine
{
	template <typename T>
		requires(std::is_enum_v<T>)
	struct EnumStringConvertor final
	{
	public:
		using self_t  = EnumStringConvertor<T>;
		using value_t = T;
		using map_t	  = const std::unordered_map<std::string_view, value_t>;

		EnumStringConvertor() = default;
		EnumStringConvertor(value_t from) : value(from)
		{
		}
		EnumStringConvertor(const std::string& from) : value(Lookup(from))
		{
		}

		operator value_t() const
		{
			return CheckedValueGetter();
		}

		operator std::string_view() const
		{
			return ReverseLookup(CheckedValueGetter());
		}

		static bool Valid(const std::string& from)
		{
			const auto& found_type = self_t::type_map.find(from);

			return (found_type != self_t::type_map.end());
		}

	private:
		static map_t type_map;

		std::optional<value_t> value;

		value_t CheckedValueGetter() const
		{
			if (value.has_value())
			{
				return value.value();
			}

			throw ENGINE_EXCEPTION(
				"Tried to get value from EnumStringConverter when value.has_value() is false!"
			);
		}

		// Finds type match from type_map
		auto Lookup(const std::string& from) const -> value_t
		{
			if (from.empty())
			{
				throw ENGINE_EXCEPTION("Cannot convert an empty string to enum");
			}

			const auto& found_type = self_t::type_map.find(from);

			if (found_type != self_t::type_map.end())
			{
				return found_type->second;
			}

			throw ENGINE_EXCEPTION("Could not convert '" + from + "' (no match found)");
		}

		// Slow linear lookup, use only on cold paths
		auto ReverseLookup(value_t from) const -> std::string_view
		{
			for (auto [key, val] : type_map)
			{
				if (val == from)
				{
					return key;
				}
			}

			return "INVALID";
		}
	};
} // namespace Engine
