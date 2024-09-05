#pragma once

#include "Factories/ObjectFactory.hpp"

#include "EnumStringConvertor.hpp"
#include "lua.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace Engine::Scripting::Utility
{
	static constexpr int lua_cdata_typeid = 10;

	std::string UnwindStack(lua_State* of_state);
	int			LuaErrorHandler(lua_State* state);

	std::string StackContentToString(lua_State* state);
	// void		PrintStackContent(lua_State* state);

	template <typename value_t = void*>
	value_t GetCData(lua_State* from_state, int idx)
		requires(std::is_pointer_v<value_t>)
	{
		return const_cast<value_t>(*reinterpret_cast<void* const*>(lua_topointer(from_state, idx)));
	}

	// Guaranteed to return null terminated string
	std::string_view MappingReverseLookup(lua_CFunction lookup_function);

	struct LuaValue final
	{
		enum class Type : uint8_t
		{
			STRING	  = LUA_TSTRING,
			NUMBER	  = LUA_TNUMBER,
			NIL		  = LUA_TNIL,
			BOOL	  = LUA_TBOOLEAN,
			// TABLE	  = LUA_TTABLE,
			USERDATA  = LUA_TUSERDATA,
			LUSERDATA = LUA_TLIGHTUSERDATA,
			CDATA	  = lua_cdata_typeid
		};

		// Tables in Lua have an array part and a hashmap part
		// they can have both parts at the same time
		// represent the entries as a variant capable of being in either part
		using table_array_key_t = int;
		using table_hash_key_t	= std::string;
		using table_entry_t =
			std::pair<std::variant<table_array_key_t, table_hash_key_t>, LuaValue>;

		using value_t = std::variant<
			std::string,
			double,
			std::nullptr_t,
			void*,
			bool /* , std::vector<table_entry_t> */>;

		const Type	  type;
		const value_t value;

		LuaValue(lua_State* state, int stack_index);

		// Basic conversion ops
		// these are checked (will throw if conversion to non-matching value is attempted)
		operator std::string() const;
		operator double() const;
		operator void*() const;
		operator bool() const;

		// Specialized conversion ops
		operator Factories::ObjectFactory::supply_data_value_t() const;
		template <typename enum_t> operator EnumStringConvertor<enum_t>() const
		{
			return static_cast<std::string>(*this);
		}
		explicit operator uintptr_t() const;

		static std::string ToString(const LuaValue& val);

		// protected:
		//	std::optional<table_entry_t> TableElemAccess(table_array_key_t key);
		//	std::optional<table_entry_t> TableElemAccess(table_hash_key_t key);
	};

} // namespace Engine::Scripting::Utility
