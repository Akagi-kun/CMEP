#define CMEP_LUAFACTORY_PUSH_TRAMPOLINE(stateL, from_mappings)                                                         \
	{                                                                                                                  \
		CreateMappingTrampoline(state, &(from_mappings));                                                              \
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
#elif defined(CMEP_USE_FACTORY_OLD_PUSH)
#	pragma message("Using old style factory mappings push")
#	define CMEP_LUAFACTORY_PUSH_MAPPINGS(stateL, from_mappings)                                                       \
		for (const auto& mapping : from_mappings)                                                                      \
		{                                                                                                              \
			lua_pushcclosure(stateL, mapping.second, 0);                                                               \
			lua_setfield(stateL, -2, mapping.first.c_str());                                                           \
		}
#else
#	pragma error "No factory mapping method selected!"
#endif
