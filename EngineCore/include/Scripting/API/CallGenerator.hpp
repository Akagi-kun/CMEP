#pragma once

#include "Scripting/API/ValueHandler.hpp"
#include "Scripting/API/framework.hpp"

#include "Detail/FunctionTraits.hpp"
#include "ValueHandler.hpp"
#include "lua.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace Engine::Scripting::API
{
/** Call function @p fn with every argument handled using Lua offsets provided by @p lua_indices */
#define LAMBDA_CALL_FUNCTION(fn, lua_indices)                                                      \
	(fn)(get_handler_t<typename traits_t::template arg<indices>>::get(                             \
		state,                                                                                     \
		(lua_indices)[indices]                                                                     \
	)...)

/**
 * Call @p fn using @ref LAMBDA_CALL_FUNCTION, if the return type is void, continue and return 0,
 * otherwise call the appropriate push handler.
 */
#define LAMBDA_RETURN_HANDLED_CALL(fn, lua_indices)                                                \
	if constexpr (std::is_same_v<typename traits_t::return_t, void>)                               \
	{                                                                                              \
		LAMBDA_CALL_FUNCTION(fn, lua_indices);                                                     \
		return 0;                                                                                  \
	}                                                                                              \
	else                                                                                           \
	{                                                                                              \
		auto return_value = LAMBDA_CALL_FUNCTION(fn, lua_indices);                                 \
		return get_handler_t<typename traits_t::return_t>::push(state, return_value);              \
	}

	/** Compile-time checks that all arguments can be handled */
	template <typename traits_t, std::size_t... indices>
	consteval void checkArgsHandleable()
	{
		static_assert(
			(ValueHandleable<typename traits_t::template arg<indices>> && ...),
			"Cannot generate lambda call that requires an unimplemented ValueHandler"
		);
	}

	/**
	 * Calculates the *first* Lua stack slot occupied by the C++ argument @p arg
	 * @param first_offset Offset all indices by this
	 */
	template <typename traits_t, size_t arg>
	consteval auto calculateLuaIndex(int first_offset)
	{
		if constexpr (arg == 0)
		{
			return 1 + first_offset;
		}
		else
		{
			return calculateLuaIndex<traits_t, arg - 1>(first_offset) +
				   static_cast<int>(get_handler_t<typename traits_t::template arg<arg - 1>>::lua_args
				   );
		}
	}

	template <std::size_t argc>
	struct LuaStackMetadata
	{
		std::array<int, argc> indices;
		int					  expected_args;
	};

	/**
	 * Creates a compile-time array that maps function arguments to Lua stack indices.
	 * @tparam traits_t Function traits to generate indices for.
	 */
	template <typename traits_t, int first_offset, std::size_t... indices>
	consteval auto calculateLuaMetadata() -> LuaStackMetadata<sizeof...(indices)>
	{
		// Create the mapping C++ arg -> Lua arg
		constexpr std::array<int, sizeof...(indices)> lua_indices = {
			calculateLuaIndex<traits_t, indices>(first_offset)...
		};

		// Index of the last C++ argument or 0
		constexpr auto last_idx = std::max<size_t>(0, sizeof...(indices));

		// Expected number of Lua stack slots occupied
		constexpr int expected_args = calculateLuaIndex<traits_t, last_idx>(first_offset) - 1;

		return {lua_indices, expected_args};
	}

	// Free functions
	template <typename traits_t, typename function_t, std::size_t... indices>
	consteval auto
	buildLambdaHelper(function_t* function, std::index_sequence<indices...> /* template only */)
	{
		checkArgsHandleable<traits_t, indices...>();

		constexpr auto lua_metadata = calculateLuaMetadata<traits_t, 0, indices...>();

		return [=](lua_State* state) -> int {
			CMEP_LUACHECK_FN_ARGC(state, lua_metadata.expected_args)

			LAMBDA_RETURN_HANDLED_CALL(function, lua_metadata.indices)
		};
	}

	// Member functions
	template <typename traits_t, typename function_t, typename class_t, std::size_t... indices>
	consteval auto
	buildLambdaHelper(function_t class_t::*function, std::index_sequence<indices...> /* template only */)
	{
		checkArgsHandleable<traits_t, indices...>();

		constexpr auto lua_metadata = calculateLuaMetadata<traits_t, 1, indices...>();

		return [=](lua_State* state) -> int {
			CMEP_LUACHECK_FN_ARGC(state, lua_metadata.expected_args)

			auto* self = get_handler_t<class_t*>::get(state, 1);

			LAMBDA_RETURN_HANDLED_CALL(self->*function, lua_metadata.indices)
		};
	}

	// Free functions
	template <typename function_t, typename traits_t = Detail::FunctionTraits<function_t>>
	consteval auto buildLambda(function_t* function)
	{
		return buildLambdaHelper<traits_t>(function, std::make_index_sequence<traits_t::arg_count>());
	}

	// Member functions
	template <
		typename function_t,
		typename class_t,
		typename traits_t = Detail::FunctionTraits<function_t>>
	consteval auto buildLambda(function_t class_t::*function)
	{
		return buildLambdaHelper<traits_t>(function, std::make_index_sequence<traits_t::arg_count>());
	}

} // namespace Engine::Scripting::API

// NOLINTBEGIN(*unused-macros)
/**
 * Define a static function (named @p wrapper_name ) that acts as a Lua-compatible wrapper
 * around @p target .
 *
 * @param wrapper_name Name of the static wrapper function.
 * @param target Target function to call, may be a member function in the form @c &MyClass::myFunction .
 */
#define GENERATED_LAMBDA_CALL_RAW(wrapper_name, target)                                            \
	static int wrapper_name(lua_State* state)                                                      \
	{                                                                                              \
		static constexpr auto lambda_fn = ::Engine::Scripting::API::buildLambda(target);           \
		return lambda_fn(state);                                                                   \
	}

/**
 * Defines a static wrapper function for a class @p class_name member @p function_name .
 */
#define GENERATED_LAMBDA_MEMBER_CALL(class_name, function_name)                                    \
	GENERATED_LAMBDA_CALL_RAW(function_name, &class_name::function_name)

/**
 * Defines a static wrapper function for free function @p function_name whose name is
 * @p function_name with @c _wrapper appended .
 */
#define GENERATED_LAMBDA_FREE_CALL(function_name)                                                  \
	GENERATED_LAMBDA_CALL_RAW(function_name##_wrapper, &(function_name))
// NOLINTEND(*unused-macros)
