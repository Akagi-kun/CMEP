#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace Engine
{
	template <typename T> struct EnumStringConvertor final
	{
		using self_type	 = EnumStringConvertor<T>;
		using value_type = T;
		using map_type	 = const std::unordered_map<std::string_view, value_type>;

		static_assert(std::is_enum<value_type>{}, "EnumStringConvertor can only convert to enum types!");

		std::optional<value_type> value;

		EnumStringConvertor(value_type from) : value(from)
		{
		}
		EnumStringConvertor(const std::string& from) : value((*this)(from))
		{
		}

		operator value_type() const
		{
			return this->value.value();
		}

	private:
		static map_type type_map;

		// Finds type match from type_map
		auto operator()(const std::string& from) const -> value_type
		{
			if (from.empty())
			{
				throw std::invalid_argument("[EnumStringConvertor] Cannot convert an empty string to enum");
			}

			const auto& found_type = this->type_map.find(from);

			if (found_type != this->type_map.end())
			{
				return found_type->second;
			}

			throw std::invalid_argument("[EnumStringConvertor] Could not convert '" + from + "' (no match found)");
		}
	};
} // namespace Engine
