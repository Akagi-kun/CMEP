#pragma once

#include "Exception.hpp"

#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace Engine
{
	template <typename enum_t>
	concept EnumType = std::is_enum_v<enum_t>;

	template <EnumType enum_t>
	struct EnumStringConvertor final
	{
	public:
		using self_t  = EnumStringConvertor<enum_t>;
		using value_t = enum_t;
		using map_t	  = const std::unordered_map<std::string_view, value_t>;

		EnumStringConvertor() = default;
		EnumStringConvertor(value_t from) : value(from) {}
		EnumStringConvertor(const std::string& from) : value(lookup(from)) {}

		/**
		 * Gets the enum value
		 */
		value_t get() const
		{
			return checkedValueGetter();
		}

		/**
		 * @sa get()
		 */
		operator value_t() const
		{
			return checkedValueGetter();
		}

		/**
		 * Tries to reverse lookup the string representing this enum value
		 * @note This is a slow operation, avoid using often
		 */
		operator std::string_view() const
		{
			return reverseLookup(checkedValueGetter());
		}

		/**
		 * Checks if a given string can be converted to an enum
		 */
		static bool valid(const std::string& from)
		{
			const auto& found_type = self_t::value_map.find(from);

			return (found_type != self_t::value_map.end());
		}

	private:
		static map_t value_map;

		std::optional<value_t> value;

		/**
		 * Returns the contained value, throws otherwise
		 */
		value_t checkedValueGetter() const
		{
			if (value.has_value())
			{
				return value.value();
			}

			throw ENGINE_EXCEPTION("Tried to get value from EnumStringConverter when "
								   "value.has_value() is false!");
		}

		/**
		 * Convert string to enum value
		 */
		auto lookup(const std::string& from) const -> value_t
		{
			if (from.empty())
			{
				throw ENGINE_EXCEPTION("Cannot convert an empty string to enum");
			}

			const auto& found_type = self_t::value_map.find(from);

			if (found_type != self_t::value_map.end())
			{
				return found_type->second;
			}

			throw ENGINE_EXCEPTION(std::format("Could not convert '{}' (no match found)", from));
		}

		/**
		 * Convert enum to string value
		 *
		 * @warning This is a slow linear lookup, use only for debug purposes (i.e. exceptions)
		 */
		auto reverseLookup(value_t from) const -> std::string_view
		{
			for (auto [key, val] : value_map)
			{
				if (val == from)
				{
					return key;
				}
			}

			throw ENGINE_EXCEPTION(
				std::format("Could not convert '{}' (no match found)", static_cast<uint32_t>(from))
			);
		}
	};

	/**
	 * Deduction guide for value-constructed @ref EnumStringConvertor.
	 */
	template <EnumType enum_t>
	EnumStringConvertor(enum_t val) -> EnumStringConvertor<enum_t>;
} // namespace Engine
