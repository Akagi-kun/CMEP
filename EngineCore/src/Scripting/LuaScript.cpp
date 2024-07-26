#include "Scripting/LuaScript.hpp"

#include "Scripting/LuaScriptExecutor.hpp"

// #include "luajit.h"

#include <stdexcept>
#include <utility>

namespace Engine::Scripting
{
	/* static void ProfilerCallback(void* data, lua_State* state, int samples, int vmstate)
	{
		auto* cast_data = static_cast<ScriptPerfState*>(data);

		size_t dump_len		   = 0;
		const char* stack_dump = luaJIT_profile_dumpstack(state, "fZ;", 5, &dump_len);

		printf("Stack:\n'");

		for (size_t i = 0; i < dump_len; i++)
		{
			fputc(stack_dump[i], stdout);
		}

		printf("'\n");

		switch (vmstate)
		{
			case 'I':
			{
				cast_data->interpreted_count++;
				break;
			}
			case 'N':
			{
				cast_data->native_count++;
				break;
			}
			case 'C':
			{
				cast_data->engine_count++;

				break;
			}
			default:
			{
				printf("Non-handled state '%c'\n", vmstate);
				break;
			}
		}
	} */

	LuaScript::LuaScript(LuaScriptExecutor* executor, std::string with_path) : path(std::move(with_path))
	{
		this->state = luaL_newstate();
		luaL_openlibs(this->state);

		this->profiler_state = new ScriptPerfState();

		// luaJIT_profile_start(this->state, "fl", &ProfilerCallback, this->profiler_state);

		int return_code = executor->LoadAndCompileScript(this);
		if (return_code != 0)
		{
			throw std::runtime_error(
				"Could not compile script '" + this->path + "' (errcode = " + std::to_string(return_code) + ")"
			);
		}
	}
} // namespace Engine::Scripting
