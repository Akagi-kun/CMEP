#include "Scripting/LuaScript.hpp"
#include "Scripting/LuaScriptExecutor.hpp"

namespace Engine::Scripting
{
	LuaScript::LuaScript(LuaScriptExecutor* executor, std::string path) : path(std::move(path))
	{
		this->state = luaL_newstate();
		luaL_openlibs(this->state);

		executor->LoadAndCompileScript(this);
	}
} // namespace Engine::Scripting