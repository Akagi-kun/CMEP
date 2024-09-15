#pragma once

#include "lua.hpp"

#include <string>
#include <string_view>

namespace Engine::Scripting::Utility
{
	static constexpr int lua_cdata_typeid = 10;

	/**
	 * @brief Unwinds a stack trace into a string
	 *
	 * Pops a singular value from the stack (index -1) that is expected to be an error message
	 *
	 * @param of_state a state that threw an error
	 * @return std::string the stack trace
	 */
	std::string unwindStack(lua_State* of_state);

	// Refer to in pcall to handle errors
	int luaErrorHandler(lua_State* state);

	/**
	 * @brief Generates a string containing the values on a Lua stack
	 *
	 * @param state Lua context from which to take the stack
	 * @param start_at first stack index to be put into the string
	 * @return the stack representation
	 */
	std::string stackContentToString(lua_State* state, int start_at = 0);

	// Use rarely
	// TODO: Better Lua/C and FFI API separation
	template <typename value_t = void*>
	value_t getCData(lua_State* state, int idx)
		requires(std::is_pointer_v<value_t>)
	{
		return const_cast<value_t>(
			*reinterpret_cast<void* const*>(lua_topointer(state, idx))
		);
	}

	/**
	 * @brief Gets the name of a function from a function pointer
	 *
	 * @param lookup_function a valid pointer to a Lua/C API function
	 * @return std::string_view name of the function (guaranteed null-terminated) or undetermined invalid
	 */
	std::string_view mappingReverseLookup(lua_CFunction lookup_function);

	/**
	 * @brief Convert a potentially relative stack index to absolute
	 *
	 * Stack modifications may render relative indices invalid
	 * use this to avoid that
	 *
	 * @param state Lua context the index is referring to
	 * @param relative a stack index (may be either positive or negative)
	 * @return an absolute stack index (positive)
	 */
	int relativeToAbsoluteIndex(lua_State* state, int relative);

} // namespace Engine::Scripting::Utility
