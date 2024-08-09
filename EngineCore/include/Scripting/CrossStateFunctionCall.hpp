#pragma once

#include "lua.hpp"

#include <string>

namespace Engine::Scripting
{
	class CrossStateFunctionCall final
	{
	private:
		static int CrossStateArgMove(lua_State* origin, lua_State* target);
		static int InternalCallFn(lua_State* caller_state);

	public:
		static void PushFunction(lua_State** from, const std::string& from_fn, lua_State* into);
	};
} // namespace Engine::Scripting
