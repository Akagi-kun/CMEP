#pragma once

#include "lua.hpp"
// #include "Scripting/lualib/lua.hpp"

#include <string>

namespace Engine::Scripting
{
	class LuaScriptExecutor;

	struct ScriptPerfState
	{
		int native_count	  = 0;
		int interpreted_count = 0;
		int engine_count	  = 0;
	};

	class LuaScript
	{
	protected:
		lua_State* state;

		ScriptPerfState* profiler_state = nullptr;

	public:
		std::string path;

		LuaScript(LuaScriptExecutor* executor, std::string with_path);
		~LuaScript()
		{
			/*
			luaJIT_profile_stop(this->state);

			printf(
				"Profiling result:\n E:%i N:%i I:%i\n",
				this->profiler_state->engine_count,
				this->profiler_state->native_count,
				this->profiler_state->interpreted_count
			); */

			delete this->profiler_state;

			lua_close(this->state);
		}

		const ScriptPerfState* GetProfilerState()
		{
			return this->profiler_state;
		}

		lua_State* GetState()
		{
			return this->state;
		}
	};
} // namespace Engine::Scripting
