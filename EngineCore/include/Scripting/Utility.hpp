#pragma once

#include "Factories/ObjectFactory.hpp"

#include "lua.hpp"

#include <string>
#include <variant>

namespace Engine::Scripting::Utility
{
	static constexpr int lua_cdata_typeid = 10;

	std::string UnwindStack(lua_State* of_state);
	int LuaErrorHandler(lua_State* state);

	void PrintStackContent(lua_State* state);

	template <typename value_t = void*> value_t GetCData(lua_State* from_state, int idx)
	{
		static_assert(std::is_pointer<value_t>());
		return const_cast<value_t>(*reinterpret_cast<void* const*>(lua_topointer(from_state, idx)));
	}

	struct LuaValue final
	{
		enum class Type : uint8_t
		{
			STRING	  = LUA_TSTRING,
			NUMBER	  = LUA_TNUMBER,
			NIL		  = LUA_TNIL,
			BOOL	  = LUA_TBOOLEAN,
			USERDATA  = LUA_TUSERDATA,
			LUSERDATA = LUA_TLIGHTUSERDATA,
			CDATA	  = lua_cdata_typeid
		};

		using value_t = std::variant<std::string, double, std::nullptr_t, void*, bool>;

		const Type type;
		const value_t value;

		LuaValue(lua_State* state, int stack_index);

		// Basic conversion ops
		// these are checked (will throw if conversion to non-matching value is attempted)
		operator std::string() const;
		operator double() const;
		operator void*() const;
		operator bool() const;

		// Specialized conversion ops
		operator Factories::ObjectFactory::valid_value_t() const;
		template <typename enum_t> operator EnumStringConvertor<enum_t>() const
		{
			return static_cast<std::string>(*this);
		}
		explicit operator uintptr_t() const;

		static std::string ToString(const LuaValue& val);
	};

	// Guaranteed to return null terminated string
	std::string_view MappingReverseLookup(lua_CFunction lookup_function);

} // namespace Engine::Scripting::Utility
