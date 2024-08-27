#include "Scripting/Utility.hpp"

#include <sstream>

namespace Engine::Scripting::Utility
{
	std::string UnwindStack(lua_State* of_state)
	{
		std::string error_msg =
			"\n--- BEGIN LUA STACK UNWIND ---\n\nError that caused this stack unwind:\n";

		std::istringstream caused_by(lua_tostring(of_state, -1));
		lua_pop(of_state, 1);

		std::string line;
		while (std::getline(caused_by, line))
		{
			error_msg.append("\t").append(line).append("\n");
		}
		error_msg.append("\n");

		int caller_level = 0;
		while (true)
		{
			lua_Debug activation_record = {};

			if (lua_getstack(of_state, caller_level, &activation_record) != 1)
			{
				break;
			}

			lua_getinfo(of_state, "nS", &activation_record);

			if (caller_level > 0)
			{
				error_msg.append("called by ");
			}
			else
			{
				error_msg.append("thrown by ");
			}

			error_msg.append(activation_record.source)
				.append(":")
				.append(std::to_string(activation_record.linedefined))
				.append("\n");

			caller_level++;
		}

		if (caller_level == 0)
		{
			error_msg.append("could not unwind stack for this error");
		}

		error_msg.append("\n--- END LUA STACK UNWIND ---\n");

		return error_msg;
	}

	int LuaErrorHandler(lua_State* state)
	{
		// Describe error by unwinding the stack
		lua_pushstring(state, UnwindStack(state).c_str());

		// UnwindStack pops the error string
		// so we return only a single string containing the stack trace
		return 1;
	}

	void PrintStackContent(lua_State* state)
	{
		for (int i = 0; i <= lua_gettop(state); i++)
		{
			// Print type of every element on stack
			printf(
				"%u %s (%s)\n",
				i,
				lua_typename(state, lua_type(state, i)),
				lua_tostring(state, i)
			);
		}
	}

} // namespace Engine::Scripting::Utility
