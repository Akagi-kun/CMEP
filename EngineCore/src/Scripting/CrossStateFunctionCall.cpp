#include "Scripting/CrossStateFunctionCall.hpp"

#include "Scripting/Utility.hpp"

#include "Exception.hpp"

#include <cstdint>
#include <format>
#include <string>

namespace Engine::Scripting
{
	static int CrossStateArgMove(lua_State* origin, lua_State* target)
	{
		// LuaJIT has no guarantees to the ID of CDATA type
		// It can be checked manually against 'lj_obj_typename' symbol
		static constexpr int cdata_type_id = 10;

		int arg_vals = 1;
		for (; arg_vals <= lua_gettop(origin); arg_vals++)
		{
			int idx_type = lua_type(origin, arg_vals);

			switch (idx_type)
			{
				// TODO: Consider adding support for userdata or tables
				case LUA_TNUMBER:
					lua_pushnumber(target, lua_tonumber(origin, arg_vals));
					break;
				case LUA_TSTRING:
					lua_pushstring(target, lua_tostring(origin, arg_vals));
					break;
				case LUA_TBOOLEAN:
					lua_pushboolean(target, lua_toboolean(origin, arg_vals));
					break;
				case LUA_TNIL:
					lua_pushnil(target);
					break;
				case cdata_type_id:
					// Beware! cross-state dependency!
					// Pushing managed memory will result in bugs when that memory is garbage
					// collected ( use ffi.C.malloc or similar explicit allocation )
					lua_pushinteger(
						target,
						*const_cast<int64_t*>(
							static_cast<const int64_t*>(lua_topointer(origin, arg_vals))
						)
					);
					break;
				default:
				{
					throw ENGINE_EXCEPTION(std::format(
						"Cannot handle type '{}' ({})",
						lua_typename(origin, idx_type),
						idx_type
					));
				}
			}
		}

		return arg_vals;
	}

	static int InternalCallFn(lua_State* caller_state)
	{
		lua_State* callee_state = *static_cast<lua_State**>(
			lua_touserdata(caller_state, lua_upvalueindex(1))
		);
		const char* callee_function_name = lua_tostring(caller_state, lua_upvalueindex(2));

		lua_settop(callee_state, 0);
		lua_getglobal(callee_state, callee_function_name);

		// Passing arguments caller -> callee
		int arg_vals = CrossStateArgMove(caller_state, callee_state);

		// Substract one from arg_vals to convert from 1-based index to 0-based count
		int ret = lua_pcall(callee_state, arg_vals - 1, LUA_MULTRET, 0);
		if (ret != LUA_OK)
		{
			throw ENGINE_EXCEPTION(std::format(
				"Exception executing a cross-state function call! lua_pcall error: {}\n{}",
				ret,
				Utility::UnwindStack(callee_state)
			));
		}

		// Passing arguments callee -> caller
		int ret_vals = CrossStateArgMove(callee_state, caller_state);

		// Substract one to convert to 0-based count
		return ret_vals - 1;
	}

	void CreateCrossStateCallBridge(lua_State** from, const std::string& from_fn, lua_State* into)
	{
		lua_pushlightuserdata(into, reinterpret_cast<void*>(from));
		lua_pushstring(into, from_fn.c_str());
		lua_pushcclosure(into, InternalCallFn, 2);
	}
} // namespace Engine::Scripting
