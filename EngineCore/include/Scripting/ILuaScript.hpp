#pragma once

#include "InternalEngineObject.hpp"
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

	class ILuaScript : public InternalEngineObject
	{
	private:
		bool enable_profiling;

	protected:
		lua_State* state;
		ScriptPerfState* profiler_state = nullptr;

		std::string path;

		int LoadAndCompileScript();

		void InitializeCall(const std::string& function);

		virtual int InternalCall(const std::string& function, void* data) = 0;

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
	};
} // namespace Engine::Scripting
