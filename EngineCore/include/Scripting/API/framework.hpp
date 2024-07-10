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
	if (self == nullptr)                                                                                               \
	{                                                                                                                  \
		return luaL_error(stateL, "Function '%s' called on an invalid object (object is nullptr)", __FUNCTION__);      \
	}

#define CMEP_LUAFACTORY_PUSH_TRAMPOLINE(stateL, from_mappings)                                                                        \
	{                                                                                                                  \
		CreateMappingTrampoline(state, &(from_mappings));                                                               \
	}

#if defined(CMEP_USE_FACTORY_NEW_PUSH)
#	pragma message("Using new style factory mappings push")
#	define CMEP_LUAFACTORY_PUSH_MAPPINGS(stateL, from_mappings)                                                       \
		for (const auto& mapping : from_mappings)                                                                      \
		{                                                                                                              \
			lua_pushstring(stateL, mapping.first.c_str());                                                             \
			lua_pushcclosure(stateL, mapping.second, 0);                                                               \
			lua_rawset(stateL, -3);                                                                                    \
		}
#elif defined(CMEP_USE_FACTORY_TRAMPOLINE)
#	pragma message("Using factory mappings trampoline")
#	define CMEP_LUAFACTORY_PUSH_MAPPINGS CMEP_LUAFACTORY_PUSH_TRAMPOLINE
#else
#	pragma message("Using old style factory mappings push")
#	define CMEP_LUAFACTORY_PUSH_MAPPINGS(stateL, from_mappings)                                                       \
		for (const auto& mapping : from_mappings)                                                                      \
		{                                                                                                              \
			lua_pushcclosure(stateL, mapping.second, 0);                                                               \
			lua_setfield(stateL, -2, mapping.first.c_str());                                                           \
		}
#endif
