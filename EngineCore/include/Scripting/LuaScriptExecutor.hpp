#pragma once

#include "lualib/lua.hpp"

#include "EventHandling.hpp"
#include "LuaScript.hpp"
#include "PlatformSemantics.hpp"

namespace Engine
{
	namespace Scripting
	{
		enum class ExecuteType {
			EventHandler,
			ObjectScript
		};

		class CMEP_EXPORT LuaScriptExecutor
		{
		protected:
			static void registerCallbacks(lua_State* state);
		public:
			LuaScriptExecutor() {};
			~LuaScriptExecutor() {};

			static void CallIntoScript(ExecuteType etype, LuaScript* script, std::string function, void* data);

			static int LoadAndCompileScript(LuaScript* script);
		};
	}
}