#include "Scripting/LuaScript.hpp"
#include "Scripting/LuaScriptExecutor.hpp"

namespace Engine::Scripting
{
	LuaScript::LuaScript(std::string path) : path(path)
	{
		this->state = luaL_newstate();
		luaL_openlibs(this->state);

		LuaScriptExecutor::LoadAndCompileScript(this);
	}
}