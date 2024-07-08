#pragma once

#define CMEP_LUAMAPPING_DEFINE(namespace, mapping) {#mapping, namespace ::mapping}
#define CMEP_CHECK_FN_ARGC(stateL, expect_args)                                                                        \
	do                                                                                                                 \
	{                                                                                                                  \
		int got_args = lua_gettop(stateL);                                                                             \
		if (got_args != expect_args)                                                                                   \
		{                                                                                                              \
			return luaL_error(                                                                                         \
				stateL,                                                                                                \
				"Invalid argument count for fn '%s' (expected %u got %u)",                                             \
				__FUNCTION__,                                                                                          \
				expect_args,                                                                                           \
				got_args                                                                                               \
			);                                                                                                         \
		}                                                                                                              \
	} while (false)
