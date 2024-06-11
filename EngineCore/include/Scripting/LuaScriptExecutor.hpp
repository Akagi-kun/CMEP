#pragma once

#include "lualib/lua.hpp"

#include "InternalEngineObject.hpp"
#include "LuaScript.hpp"

namespace Engine::Scripting
{
	enum class ExecuteType
	{
		EventHandler
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
} // namespace Engine::Scripting