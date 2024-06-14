#pragma once

#include "Scripting/lualib/lua.hpp"

#include <string>

namespace Engine::Scripting
{
	class LuaScriptExecutor;

	class LuaScript
	{
	protected:
		lua_State* state;

	public:
		std::string path;

		LuaScript(LuaScriptExecutor* executor, std::string path);
		~LuaScript()
		{
			lua_close(this->state);
		}

		lua_State* GetState()
		{
			return this->state;
		}
	};
} // namespace Engine::Scripting
