#pragma once

#include "lua.hpp"

#include <string>

namespace Engine::Scripting
{
	/**
	 * @todo Move to utility or similar
	 */
	void
	createCrossStateCallBridge(lua_State** from, const std::string& from_fn, lua_State* into);
} // namespace Engine::Scripting
