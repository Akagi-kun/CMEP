#pragma once

#include "Scripting/LuaValue.hpp"

#include "Exception.hpp"
#include "lua.hpp"

namespace Engine::Scripting
{
	template <typename class_t>
	class_t* getObjectAsPointer(lua_State* state, int index)
	{
		auto val = UNWRAP(LuaValue(state, index)["_ptr"]);

		auto* val_ptr = CHECK(static_cast<class_t*>(val));

		return val_ptr;
	}
} // namespace Engine::Scripting
// NOLINTBEGIN(*unused-macros)
/**
 * Defines a mapping pair for the specified function
 *
 * These are in the form {"myFunctionName", myFunctionPtr}
 * @note Preferrably put API functions into an anonymous namespace
 *
 * @param mapping A function name
 */
#define CMEP_LUAMAPPING_DEFINE(mapping) {#mapping, mapping}

/**
 * Quick macro that checks whether the correct number of arguments was supplied
 *
 * @warning Does not check whether the types and values of arguments are correct!
 *
 * @param stateL The state supplied to the caller
 * @param expect_args The number of arguments this function expects
 */
#define CMEP_LUACHECK_FN_ARGC(stateL, expect_args)                                                 \
	{                                                                                              \
		int got_args = lua_gettop(stateL);                                                         \
		EXCEPTION_ASSERT(                                                                          \
			(expect_args) == got_args,                                                             \
			std::format("Invalid argument count (expected {} got {})", (expect_args), got_args)    \
		);                                                                                         \
	}

/**
 * Quickly get the "self" pointer for the Lua API
 *
 * @param stateL The state supplied to the caller
 * @param type The type of the "self" pointer
 *
 * @note This macro does not return the pointer instead it creates a new symbol
 *       @code{.cpp} type* self; @endcode in the namespace it was called from
 */
#define CMEP_LUAGET_PTR(state, type)                                                               \
	auto* self = ::Engine::Scripting::getObjectAsPointer<type>(state, 1);
// NOLINTEND(*unused-macros)
