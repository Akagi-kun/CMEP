#pragma once

#include "Scripting/Utility.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EnumStringConvertor.hpp"
#include "lua.hpp"

#include <cstddef>
#include <cstdint>
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

		using string_t = std::string;
		using ptr_t	   = void*;
		using number_t = double;

		// nil is not a valid key!
		using table_entry_t = std::pair<LuaValue, LuaValue>;
		using table_t		= std::vector<table_entry_t>;

		using value_t =
			std::variant<string_t, number_t, std::nullptr_t, ptr_t, bool, std::vector<table_entry_t>>;

		const Type	  type;
		const value_t value;

		// Value constructors
		LuaValue(std::nullptr_t);
		LuaValue(string_t val);
		LuaValue(number_t val);

		/**
		 * @brief Stack constructor
		 *
		 * @param state state from which to take the value
		 * @param stack_index index in the state's stack
		 */
		LuaValue(lua_State* state, int stack_index);

		// Basic getters
		// these are checked (will throw if conversion to non-matching value is attempted)
		operator string_t() const; // Type::STRING
		operator number_t() const; // Type::NUMBER
		operator ptr_t() const;	   // CDATA/(L)USERDATA
		operator bool() const;	   // Type::BOOL

		// Table related
		// Index operator
		// throws if value is non-table
		template <typename val_compat_t>
		LuaValue operator[](const val_compat_t& key_val)
			requires(std::is_same_v<val_compat_t, LuaValue> || std::is_constructible_v<LuaValue, val_compat_t>)
		{
			return getTableValue(LuaValue(key_val));
		}
		// Converts the value to one compatible with range-based for loops
		// throws if value is non-table
		[[nodiscard]] table_t toTable() const;
		[[nodiscard]] size_t  size() const;

		// Specialized getters
		template <typename enum_t> operator EnumStringConvertor<enum_t>() const
		{
			return static_cast<std::string>(*this);
		}
		explicit operator uintptr_t() const;
		explicit operator Factories::ObjectFactory::supply_data_value_t() const;

		// Utility operators
		bool operator==(const LuaValue& other) const;

		// Utility functions
		/**
		 * @brief Converts a LuaValue into it's string representation
		 *
		 * @note This may not correspond to the equivalent
		 *       Lua code necessary to produce this value
		 *
		 * @note In most cases this will be similar or equivalent to doing
		 *       @code{.lua}
		 *       	print(val)
		 *       @endcode in Lua
		 *
		 * @param val the LuaValue to convert
		 * @return std::string it's string representation
		 */
		static std::string toString(const LuaValue& val);

	protected:
		[[nodiscard]] LuaValue getTableValue(const LuaValue& key) const;
	};
} // namespace Engine::Scripting
