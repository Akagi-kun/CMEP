/**
 * @file
 * @brief Defines LuaValue, an utility class for the Lua <-> C API.
 */
#pragma once

#include "Scripting/Utility.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EnumStringConvertor.hpp"
#include "lua.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Engine::Scripting
{
	struct LuaValue final
	{
	public:
		enum class Type : uint8_t
		{
			STRING	  = LUA_TSTRING,
			NUMBER	  = LUA_TNUMBER,
			NIL		  = LUA_TNIL,
			BOOL	  = LUA_TBOOLEAN,
			TABLE	  = LUA_TTABLE,
			USERDATA  = LUA_TUSERDATA,
			LUSERDATA = LUA_TLIGHTUSERDATA,
			CDATA	  = Utility::lua_cdata_typeid
		};

		using string_t = std::string; /**< Type containing a Lua string. */
		using ptr_t	   = void*;		  /**< Type containing a pointer-like value. */
		using number_t = double;	  /**< Type containing a Lua number. */

		/**
		 * Type containing a single entry in a Lua Table.
		 * @warning Using Type::NIL as the key is invalid!
		 */
		using table_entry_t = std::pair<LuaValue, LuaValue>;
		using table_t		= std::vector<table_entry_t>; /**< Type containing a Lua table. */

		using value_t =
			std::variant<string_t, number_t, std::nullptr_t, ptr_t, bool, std::vector<table_entry_t>>;

		const Type type;	 /**< Type of this Lua value. */
		const value_t value; /**< Raw Lua value. Use `operator <type>()` instead of using this raw. */

		/** Construct a LuaValue object as if containing a Lua `nil` value. */
		LuaValue(std::nullptr_t) : type(Type::NIL), value(nullptr) {}
		/** @copydoc LuaValue(std::nullptr_t) */
		LuaValue() : LuaValue(nullptr) {}

		/** Construct a LuaValue object as if containing a Lua `string` value. */
		LuaValue(string_t val) : type(Type::STRING), value(val) {}
		/** Construct a LuaValue object as if containing a Lua `number` value. */
		LuaValue(number_t val) : type(Type::NUMBER), value(val) {}

		/**
		 * Construct a LuaValue object from an actual Lua value on the stack.
		 *
		 * @param state State from which to take the value.
		 * @param stack_index Index in the state's stack.
		 */
		LuaValue(lua_State* state, int stack_index);

		/** @{ Convert to desired type, throw if @ref type is incompatible. */
		operator string_t() const;
		operator number_t() const;
		operator ptr_t() const;
		operator bool() const;
		/** @} */

		/**
		 * Index operator for tables. Invalid to call if @ref type is not @ref Type::TABLE.
		 *
		 * @tparam val_compat_t @ref LuaValue compatible type.
		 *
		 * @param key_val Value to index the table with (the key).
		 * @return The value at the specified key, empty optional otherwise.
		 */
		template <typename val_compat_t>
		std::optional<LuaValue> operator[](const val_compat_t& key_val)
			requires(std::is_same_v<val_compat_t, LuaValue> || std::is_constructible_v<LuaValue, val_compat_t>)
		{
			return getTableValue(LuaValue(key_val));
		}
		/**
		 * Convert to a table-like object.
		 * Can be used to implement range-based for loops like:
		 * @code
		 *	for (const auto& [key, val] : table.toTable())
		 *	{
		 *		// Do something with key and val
		 *	}
		 * @endcode
		 */
		[[nodiscard]] table_t toTable() const;
		[[nodiscard]] size_t  size() const;

		// Specialized getters
		template <typename enum_t> operator EnumStringConvertor<enum_t>() const
		{
			return static_cast<std::string>(*this);
		}
		explicit operator uintptr_t() const;
		explicit operator Factories::ObjectFactory::supply_data_value_t() const;

		/**
		 * Compare against @p other.
		 *
		 * @param[in] other Value to compare against.
		 */
		bool operator==(const LuaValue& other) const;
		bool operator!=(const LuaValue& other) const; /**< @copydoc operator==() */

		/**
		 * Converts a LuaValue into it's string representation
		 *
		 * @note This may not correspond to the equivalent
		 *       Lua code necessary to produce this value
		 *
		 * @note In most cases this will be similar or equivalent to doing
		 *       @code{.lua}
		 *       	print(val)
		 *       @endcode in Lua.
		 *
		 * @param val Value to convert
		 * @return The string representation
		 */
		static std::string toString(const LuaValue& val);

	protected:
		[[nodiscard]] std::optional<LuaValue> getTableValue(const LuaValue& key) const;
	};
} // namespace Engine::Scripting
