#pragma once

#include "lualib/lua.hpp"

#include "EventHandling.hpp"
#include "LuaScript.hpp"

namespace Engine
{
	namespace Scripting
	{
		enum class ExecuteType {
			EventHandler,
			ObjectScript
		};

		class __declspec(dllexport) LuaScriptExecutor
		{
		protected:
			static void registerCallbacks(lua_State* state);
		public:
			LuaScriptExecutor() {};
			~LuaScriptExecutor() {};

			static void ConfigScriptLoad(LuaScript* script);

			static void CallIntoScript(ExecuteType etype, LuaScript* script, std::string function, void* data);

			static int LoadAndCompileScript(LuaScript* script);
		};
	}
}