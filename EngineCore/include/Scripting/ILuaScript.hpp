#pragma once

#include "InternalEngineObject.hpp"
#include "lua.hpp"

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

	std::string UnwindStack(lua_State* of_state);
	int LuaErrorHandler(lua_State* state);

	class ILuaScript : public InternalEngineObject
	{
	public:
		ILuaScript(Engine* with_engine, std::string with_path, bool with_enable_profiling = false);
		virtual ~ILuaScript();

		int CallFunction(const std::string& function, void* data);

		[[nodiscard]] const ScriptPerfState* GetProfilerState() const
		{
			return this->profiler_state;
		}

		[[nodiscard]] lua_State* GetState()
		{
			return this->state;
		}

		[[nodiscard]] std::string_view GetPath() const
		{
			return this->path;
		}

	protected:
		lua_State* state;
		ScriptPerfState* profiler_state = nullptr;

		std::string path;

		int LoadAndCompileScript();

		virtual int InternalCall(const std::string& function, void* data) = 0;

	private:
		const bool enable_profiling;
	};
} // namespace Engine::Scripting
