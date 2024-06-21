#pragma once

#include "InternalEngineObject.hpp"
#include "LuaScript.hpp"
#include "lua.hpp"
// #include "lualib/lua.hpp"

namespace Engine::Scripting
{
	enum class ExecuteType
	{
		MIN_ENUM = 0x00,

		EVENT_HANDLER = 2,

		MAX_ENUM = 0xFF
	};

	class LuaScriptExecutor : public InternalEngineObject
	{
	protected:
		static void RegisterCallbacks(lua_State* state);

		void RegisterMeta(lua_State* state);

	public:
		LuaScriptExecutor()	 = default;
		~LuaScriptExecutor() = default;

		int CallIntoScript(
			ExecuteType etype,
			const std::shared_ptr<LuaScript>& script,
			const std::string& function,
			void* data
		);

		int LoadAndCompileScript(LuaScript* script);
	};
} // namespace Engine::Scripting
