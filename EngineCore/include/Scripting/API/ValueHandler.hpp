#pragma once

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"
#include "Scripting/LuaValue.hpp"

#include "EnumStringConvertor.hpp"
#include "Exception.hpp"
#include "lua.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>

namespace Engine::Scripting::API
{
#define SHOULDNT_IMPLEMENT static_assert(false)

	// NOLINTBEGIN(bugprone-macro-parentheses)

/** Define a getter with specified parameter names */
#define DEFINE_GETTER_NAMEDPARAM(state_name, idx_name)                                             \
	static target_t get(lua_State* state_name, int idx_name)

/** Use to define a `get()` function for @ref Engine::Scripting::API::ValueHandler */
#define DEFINE_GETTER_IMPL DEFINE_GETTER_NAMEDPARAM(state, idx)

/** Use to define a `get()` function that is present for compliance, but won't compile if ever called */
#define DEFINE_GETTER_NOIMPL                                                                       \
	DEFINE_GETTER_NAMEDPARAM(, )                                                                   \
	{                                                                                              \
		SHOULDNT_IMPLEMENT;                                                                        \
	}

/** Same as @ref DEFINE_GETTER_NAMEDPARAM but for `push()` */
#define DEFINE_PUSHER_NAMEDPARAM(state_name, target_name)                                          \
	static int push(lua_State* state_name, target_t target_name)

/** Same as @ref DEFINE_GETTER_IMPL but for `push()` */
#define DEFINE_PUSHER_IMPL DEFINE_PUSHER_NAMEDPARAM(state, target)

/** Same as @ref DEFINE_GETTER_NOIMPL but for `push()` */
#define DEFINE_PUSHER_NOIMPL                                                                       \
	DEFINE_PUSHER_NAMEDPARAM(, )                                                                   \
	{                                                                                              \
		SHOULDNT_IMPLEMENT;                                                                        \
	}

	// NOLINTEND(bugprone-macro-parentheses)

	/**
	 * A common point to handle different types needed when generating lambdas.
	 * Can be easily extended to include more types.
	 */
	template <typename value_t>
	struct ValueHandler
	{
		using target_t = value_t;

		/**
		 * Number of Lua arguments that equal one C++ argument of a specific type.
		 * Used to calculate offsets into the Lua stack.
		 */
		static constexpr size_t lua_args = 1;

		/**
		 * get functions have the following constraints:
		 *
		 * 1. Take in lua_State* and int index
		 * 2. Index points to the first argument that should be handled
		 *    Offset can be used to handle additional arguments if necessary,
		 *    maximum offset is @p lua_args
		 * 3. Stack can be modified but should be in it's original state when exitting the function
		 * 4. Return a value of the target type
		 */
		DEFINE_GETTER_IMPL
		{
			return static_cast<target_t>(LuaValue(state, idx));
		}

		/**
		 * push functions have the following constraints:
		 *
		 * 1. Take in lua_State* and exactly one parameter of the target type
		 * 2. Push the target value in a way that is symmetrical with the associated `get` call
		 * 3. Stack can be modified, but should be in it's original state and the values pushed when
		 *    exitting the function
		 * 4. Return the number of values pushed into the Lua stack
		 */
		DEFINE_PUSHER_IMPL
		{
			LuaValue(target).push(state);
			return 1;
		}
	};

	template <typename value_t>
	using handler_compatible_t = std::remove_cv_t<std::remove_reference_t<value_t>>;

	template <typename value_t>
	using get_handler_t = ValueHandler<handler_compatible_t<value_t>>;

	//
	// Place specializations after this comment
	//

	template <typename element_t>
	struct ValueHandler<glm::vec<2, element_t>>
	{
		using target_t					 = glm::vec<2, element_t>;
		static constexpr size_t lua_args = 2;

		DEFINE_GETTER_IMPL
		{
			return {
				static_cast<LuaValue::number_t>(LuaValue(state, idx)),
				static_cast<LuaValue::number_t>(LuaValue(state, idx + 1))
			};
		}

		DEFINE_PUSHER_IMPL
		{
			LuaValue(target.x).push(state);
			LuaValue(target.y).push(state);
			return 2;
		}
	};

	template <typename element_t>
	struct ValueHandler<glm::vec<3, element_t>>
	{
		using target_t					 = glm::vec<3, element_t>;
		static constexpr size_t lua_args = 3;

		DEFINE_GETTER_IMPL
		{
			return {
				static_cast<LuaValue::number_t>(LuaValue(state, idx)),
				static_cast<LuaValue::number_t>(LuaValue(state, idx + 1)),
				static_cast<LuaValue::number_t>(LuaValue(state, idx + 2))
			};
		}

		DEFINE_PUSHER_IMPL
		{
			LuaValue(target.x).push(state);
			LuaValue(target.y).push(state);
			LuaValue(target.z).push(state);
			return 3;
		}
	};

	template <EnumType enum_t>
	struct ValueHandler<enum_t>
	{
		using target_t					 = enum_t;
		static constexpr size_t lua_args = 1;

		DEFINE_GETTER_IMPL
		{
			return EnumStringConvertor<target_t>(
				static_cast<LuaValue::string_t>(LuaValue(state, idx))
			);
		}

		DEFINE_PUSHER_NOIMPL
	};

	template <typename object_t>
	struct ValueHandler<object_t*>
	{
		using target_t					 = object_t*;
		static constexpr size_t lua_args = 1;

		DEFINE_GETTER_IMPL
		{
			return getObjectAsPointer<object_t>(state, idx);
		}

		DEFINE_PUSHER_IMPL
		{
			API::LuaFactories::templatedFactory<object_t>(state, target);
			return 1;
		}
	};

	template <typename wrapped_t>
	struct ValueHandler<std::shared_ptr<wrapped_t>>
	{
		using target_t					 = std::shared_ptr<wrapped_t>;
		static constexpr size_t lua_args = 1;

		DEFINE_GETTER_NOIMPL

		DEFINE_PUSHER_IMPL
		{
			// Push as a raw pointer
			return get_handler_t<wrapped_t*>::push(state, target.get());
		}
	};

	template <typename wrapped_t>
	struct ValueHandler<std::weak_ptr<wrapped_t>>
	{
		using target_t					 = std::weak_ptr<wrapped_t>;
		static constexpr size_t lua_args = 1;

		DEFINE_GETTER_NOIMPL

		DEFINE_PUSHER_IMPL
		{
			return get_handler_t<std::shared_ptr<wrapped_t>>::push(state, CHECK(target));
		}
	};

	/**
	 * Check that some @p value_t is handleable by the call generator.
	 * Use `handler_compatible_t` first to convert type into the proper form.
	 */
	template <typename value_t>
	concept ValueHandleable = requires {
		typename get_handler_t<value_t>::target_t;
		get_handler_t<value_t>::lua_args;
		get_handler_t<value_t>::get;
		get_handler_t<value_t>::push;
	};
} // namespace Engine::Scripting::API
