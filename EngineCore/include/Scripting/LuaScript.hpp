#pragma once

#include <string>

#include "Scripting/lualib/lua.hpp"
#include "Logging/Logging.hpp"

namespace Engine::Scripting
{
	class LuaScriptExecutor;

	class __declspec(dllexport) LuaScript
	{
	protected:
		lua_State* state;

	public:
		std::string path;

		LuaScript(std::string path);
		~LuaScript()
		{
			lua_close(this->state);
		}

		lua_State* GetState() { return this->state; }
	};
}