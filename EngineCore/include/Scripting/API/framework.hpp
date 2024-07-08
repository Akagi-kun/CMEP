#pragma once

#define CMEP_LUAMAPPING_DEFINE(namespace, mapping) {#mapping, namespace ::mapping}
#define CMEP_LUACHECK_FN_ARGC(stateL, expect_args)                                                                     \
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
	}

#define CMEP_LUAGET_PTR(stateL, type)                                                                                  \
	lua_getfield(stateL, 1, "_ptr");                                                                                   \
	auto* self = static_cast<type*>(lua_touserdata(state, -1));                                                        \
	assert(self != nullptr);

#define CMEP_LUAFACTORY_PUSH_MAPPINGS(stateL, from_mappings)                                                           \
	for (const auto& mapping : from_mappings)                                                                          \
	{                                                                                                                  \
		lua_pushcfunction(stateL, mapping.second);                                                                     \
		lua_setfield(stateL, -2, mapping.first.c_str());                                                               \
	}
