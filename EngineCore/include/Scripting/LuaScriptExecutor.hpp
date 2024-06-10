#pragma once

#include "lualib/lua.hpp"

#include "EventHandling.hpp"
#include "LuaScript.hpp"
#include "PlatformSemantics.hpp"
#include "InternalEngineObject.hpp"

namespace Engine
{
	namespace Scripting
	{
		enum class ExecuteType {
			EventHandler,
			ObjectScript
		};

		class LuaScriptExecutor : public InternalEngineObject
		{
		protected:
			static void RegisterCallbacks(lua_State* state);
		
			void RegisterMeta(lua_State* state);
		public:
			LuaScriptExecutor() {};
			~LuaScriptExecutor() {};

			int CallIntoScript(ExecuteType etype, std::shared_ptr<LuaScript> script, std::string function, void* data);

			int LoadAndCompileScript(LuaScript* script);
		};
	}
}