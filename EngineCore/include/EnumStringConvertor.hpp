#pragma once

#include <map>
#include <string>
#include <type_traits>

namespace Engine
{
	template <typename T> struct EnumStringConvertor
	{
		using self_type	 = EnumStringConvertor<T>;
		using value_type = T;
		using map_type	 = const std::map<std::string, value_type>;

		static_assert(std::is_enum<value_type>{}, "EnumStringConvertor can only convert to enum types!");
		static_assert(
			std::is_scalar<decltype(value_type::MIN_ENUM)>{} && std::is_scalar<decltype(value_type::MAX_ENUM)>{},
			"EnumStringConvertor can only convert to enum types with defined MIN_ENUM and MAX_ENUM values!"
		);

		value_type value;

		EnumStringConvertor() = default;
		EnumStringConvertor(value_type from) : value(from)
		{
		}
		EnumStringConvertor(const std::string& from) : value((*this)(from))
		{
		}

		operator value_type() const
		{
			return this->value;
		}

	private:
		static map_type type_map;

		// Finds type match from type_map
		auto operator()(const std::string& from) const -> value_type
		{
			if (from.empty())
			{
				return value_type::MIN_ENUM;
			}

			const auto& found_type = this->type_map.find(from);

			if (found_type != this->type_map.end())
			{
				return found_type->second;
			}

			return value_type::MAX_ENUM;
		}
	};
} // namespace Engine
