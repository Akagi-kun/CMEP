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
	public:
		using self_t  = EnumStringConvertor<T>;
		using value_t = T;
		using map_t	  = const std::unordered_map<std::string_view, value_t>;

		static_assert(std::is_enum<value_t>{}, "EnumStringConvertor can only convert to enum types!");

		std::optional<value_t> value;

		EnumStringConvertor() = default;
		EnumStringConvertor(value_t from) : value(from)
		{
		}
		EnumStringConvertor(const std::string& from) : value((*this)(from))
		{
		}

		operator value_t() const
		{
			return this->value.value();
		}

		static bool Valid(const std::string& from)
		{
			const auto& found_type = self_t::type_map.find(from);

			return (found_type != self_t::type_map.end());
		}

	private:
		static map_t type_map;

		// Finds type match from type_map
		auto operator()(const std::string& from) const -> value_t
		{
			if (from.empty())
			{
				throw std::invalid_argument("[EnumStringConvertor] Cannot convert an empty string to enum");
			}

			const auto& found_type = self_t::type_map.find(from);

			if (found_type != self_t::type_map.end())
			{
				return found_type->second;
			}

			throw std::invalid_argument("[EnumStringConvertor] Could not convert '" + from + "' (no match found)");
		}
	};
} // namespace Engine
