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
		{}
		EnumStringConvertor(const std::string& from) : value(lookup(from))
		{}

		operator value_t() const
		{
			return checkedValueGetter();
		}

		operator std::string_view() const
		{
			return reverseLookup(checkedValueGetter());
		}

		static bool valid(const std::string& from)
		{
			const auto& found_type = self_t::value_map.find(from);

			return (found_type != self_t::value_map.end());
		}

	private:
		static map_t value_map;

		std::optional<value_t> value;

		/**
		 * @brief Returns the contained value, throws otherwise
		 */
		value_t checkedValueGetter() const
		{
			if (value.has_value()) { return value.value(); }

			throw ENGINE_EXCEPTION("Tried to get value from EnumStringConverter when "
								   "value.has_value() is false!");
		}

		/**
		 * @brief string to enum value
		 */
		auto lookup(const std::string& from) const -> value_t
		{
			if (from.empty())
			{
				throw ENGINE_EXCEPTION("Cannot convert an empty string to enum");
			}

			const auto& found_type = self_t::value_map.find(from);

			if (found_type != self_t::value_map.end()) { return found_type->second; }

			throw ENGINE_EXCEPTION(
				std::format("Could not convert '{}' (no match found)", from)
			);
		}

		/**
		 * @brief enum to string value
		 *
		 * @warning This is a slow linear lookup, use only for debug purposes (i.e. exceptions)
		 */
		auto reverseLookup(value_t from) const -> std::string_view
		{
			for (auto [key, val] : value_map)
			{
				if (val == from) { return key; }
			}

			throw ENGINE_EXCEPTION(std::format(
				"Could not convert '{}' (no match found)",
				static_cast<uint32_t>(from)
			));
		}
	};

	/**
	 * @brief Deduction guide for value-constructed @ref EnumStringConvertor.
	 */
	template <typename T> EnumStringConvertor(T val) -> EnumStringConvertor<T>;
} // namespace Engine
