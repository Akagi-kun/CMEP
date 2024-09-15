#include "Scripting/Utility.hpp"

#include "Scripting/API/AssetManager_API.hpp"
#include "Scripting/API/Engine_API.hpp"
#include "Scripting/API/Global_API.hpp"
#include "Scripting/API/Object_API.hpp"
#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/Scene_API.hpp"
#include "Scripting/LuaValue.hpp"

#include <cassert>
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Engine::Scripting::Utility
{
	std::string unwindStack(lua_State* of_state)
	{
		std::string error_msg = "--- BEGIN LUA STACK UNWIND ---\n\nError that caused "
								"this stack unwind:\n";

		// Cause
		std::istringstream caused_by(lua_tostring(of_state, -1));
		lua_pop(of_state, 1);

		std::string line;
		while (std::getline(caused_by, line))
		{
			error_msg.append(std::format("\t{}\n", line));
		}
		error_msg.append("\n");
		// Cause

		// Stack trace
		int caller_level = 0;
		while (true)
		{
			lua_Debug activation_record = {};

			if (lua_getstack(of_state, caller_level, &activation_record) != 1) { break; }

			lua_getinfo(of_state, "nS", &activation_record);

			bool is_internal = activation_record.source[0] == '=';

			// The error was thrown (level = 0) inside the engine (is_internal = true)
			if (is_internal && caller_level == 0)
			{
				error_msg.append("thrown (internally)\n");
			}
			// Skip internal calls (shorter stack trace)
			else if (!is_internal)
			{
				std::string_view action = caller_level > 0 ? "called" : "thrown";

				assert(strlen(activation_record.source) > 1);

				error_msg.append(std::format(
					"{} by '{}':{}\n",
					action,
					&activation_record.source[1],
					activation_record.linedefined
				));
			}

			caller_level++;
		}
		// Stack trace

		// caller_level will be higher than 0
		// if any stack records could be unwound
		if (caller_level == 0)
		{
			error_msg.append("could not unwind stack for this error");
		}

		error_msg.append("\n--- END LUA STACK UNWIND ---");

		return error_msg;
	}

	int luaErrorHandler(lua_State* state)
	{
		// Describe error by unwinding the stack
		lua_pushstring(state, unwindStack(state).c_str());

		// UnwindStack pops the error string
		// so we return only a single string containing the stack trace
		return 1;
	}

	[[nodiscard]] std::string stackContentToString(lua_State* state, int start_at)
	{
		std::string output;

		for (int i = start_at; i <= lua_gettop(state); i++)
		{
			auto val = LuaValue(state, i);

			/* output.append(std::format(
				"{} {} ({})\n",
				i,
				lua_typename(state, lua_type(state, i)),
				lua_tostring(state, i)
			)); */

			output.append(std::format("{} {}\n", i, LuaValue::toString(val)));
		}

		return output;
	}

	std::string_view mappingReverseLookup(lua_CFunction lookup_function)
	{
		static const std::vector<std::unordered_map<std::string, const lua_CFunction>>
			all_mappings = {
				API::global_mappings,
				API::object_mappings,
				API::scene_manager_mappings,
				API::asset_manager_mappings,
				API::scene_mappings,
				API::engine_mappings
			};

		for (const auto& mapping : all_mappings)
		{
			for (const auto& [name, function] : mapping)
			{
				if (function == lookup_function) { return name; }
			}
		}

		return "[no match found]";
	}

	int relativeToAbsoluteIndex(lua_State* state, int relative)
	{
		// Return unchanged if != relative
		// else convert to absolute by adding length of stack + 1
		return (relative > 0 || relative <= LUA_REGISTRYINDEX)
				   ? relative
				   : lua_gettop(state) + relative + 1;
	}

} // namespace Engine::Scripting::Utility
