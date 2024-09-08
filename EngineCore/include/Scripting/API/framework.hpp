#pragma once

// #define CMEP_LUAMAPPING_DEFINE(namespace, mapping) {#mapping, namespace ::mapping}
#define CMEP_LUAMAPPING_DEFINE(mapping) {#mapping, mapping}

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

#define CMEP_LUAGET_PTR(stateL, type)                                                    \
	lua_getfield(stateL, 1, "_ptr");                                                     \
	auto* self = static_cast<type*>(lua_touserdata(state, -1));                          \
	if (self == nullptr)                                                                 \
	{                                                                                    \
		return luaL_error(                                                               \
			stateL,                                                                      \
			"Function '%s' called on an invalid object (object is nullptr)",             \
			__FUNCTION__                                                                 \
		);                                                                               \
	}
