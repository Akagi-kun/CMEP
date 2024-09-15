#pragma once

/**
 * @brief Defines a mapping pair for the specified function
 *
 * These are in the form {"myFunctionName", myFunctionPtr}
 * @note Preferrably put API functions into an anonymous namespace
 *
 * @param mapping A function name
 */
#define CMEP_LUAMAPPING_DEFINE(mapping) {#mapping, mapping}

/**
 * @brief Quick macro that checks whether the correct number of arguments was supplied
 *
 * @warning Does not check whether the types and values of arguments are correct!
 *
 * @param stateL the state supplied to the caller
 * @param expect_args the number of arguments this function expects
 */
#define CMEP_LUACHECK_FN_ARGC(stateL, expect_args)                                       \
	{                                                                                    \
		int got_args = lua_gettop(stateL);                                               \
		if (got_args != expect_args)                                                     \
		{                                                                                \
			return luaL_error(                                                           \
				stateL,                                                                  \
				"Invalid argument count for fn '%s' (expected %u got %u)",               \
				__FUNCTION__,                                                            \
				expect_args,                                                             \
				got_args                                                                 \
			);                                                                           \
		}                                                                                \
	}

/**
 * @brief Quickly get the "self" pointer for the Lua API
 *
 * @param stateL the state supplied to the caller
 * @param type the type of the "self" pointer
 *
 * @return @code{.cpp}
 *         		type* self;
 *         @endcode
 */
#define CMEP_LUAGET_PTR(stateL, type)                                                    \
	lua_getfield(stateL, 1, "_ptr");                                                     \
	type* self = static_cast<type*>(lua_touserdata(stateL, -1));                         \
	if (self == nullptr)                                                                 \
	{                                                                                    \
		return luaL_error(                                                               \
			stateL,                                                                      \
			"Function '%s' called on an invalid object (object is nullptr)",             \
			__FUNCTION__                                                                 \
		);                                                                               \
	}
