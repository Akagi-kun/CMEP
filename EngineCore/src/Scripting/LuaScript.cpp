#include "Scripting/LuaScript.hpp"

#include "Scripting/LuaScriptExecutor.hpp"

#include <stdexcept>
#include <utility>

namespace Engine::Scripting
{
	LuaScript::LuaScript(LuaScriptExecutor* executor, std::string with_path) : path(std::move(with_path))
	{
		this->state = luaL_newstate();
		luaL_openlibs(this->state);

		int return_code = executor->LoadAndCompileScript(this);
		if (return_code != 0)
		{
			throw std::runtime_error(
				"Could not compile script '" + this->path + "' (errcode = " + std::to_string(return_code) + ")"
			);
		}
	}
} // namespace Engine::Scripting
