#pragma once

#include <cstddef>
#include <tuple>

namespace Engine::Detail
{
	// Common code for all FunctionTraits
	template <typename ret_t, typename... args_t>
	struct FunctionTraitsImpl
	{
		using return_t = ret_t;
		using tuple_t  = std::tuple<args_t...>;

		template <std::size_t idx>
		using arg = typename std::tuple_element<idx, tuple_t>::type;
		// arg<0> is the first argument to the function

		static constexpr std::size_t arg_count = sizeof...(args_t);
	};

	/**
	 * Gets useful metadata of a function type that can aid in template metaprogramming.
	 *
	 * @tparam function_t Function type to get traits of
	 */
	template <typename function_t>
	struct FunctionTraits;

	// Specialization for pure function types
	template <typename ret_t, typename... args_t>
	struct FunctionTraits<ret_t(args_t...)> : FunctionTraitsImpl<ret_t, args_t...>
	{
	};

	// Specialization for pure function types (const)
	template <typename ret_t, typename... args_t>
	struct FunctionTraits<ret_t(args_t...) const> : FunctionTraitsImpl<ret_t, args_t...>
	{
	};

	// Specialization for pure function types (noexcept)
	template <typename ret_t, typename... args_t>
	struct FunctionTraits<ret_t(args_t...) noexcept> : FunctionTraitsImpl<ret_t, args_t...>
	{
	};

	// Specialization for pure function types (const noexcept)
	template <typename ret_t, typename... args_t>
	struct FunctionTraits<ret_t(args_t...) const noexcept> : FunctionTraitsImpl<ret_t, args_t...>
	{
	};

#ifdef CURRENTLY_UNUSED
	/*
	// Specialization for free function pointers
	template <typename ret_t, typename... arg_pack_t>
	struct FunctionTraits<ret_t (*)(arg_pack_t...)>
		: public FunctionTraitsImpl<ret_t, arg_pack_t...>
	{};

	// Specialization for member function pointers
	template <typename class_t, typename ret_t, typename... arg_pack_t>
	struct FunctionTraits<ret_t (class_t::*)(arg_pack_t...)>
		: public FunctionTraitsImpl<ret_t, arg_pack_t...>
	{};

	// Specialization for member function pointers (const)
	template <typename class_t, typename ret_t, typename... arg_pack_t>
	struct FunctionTraits<ret_t (class_t::*)(arg_pack_t...) const>
		: public FunctionTraitsImpl<ret_t, arg_pack_t...>
	{};

	// Specialization for member function pointers (noexcept)
	template <typename class_t, typename ret_t, typename... arg_pack_t>
	struct FunctionTraits<ret_t (class_t::*)(arg_pack_t...) noexcept>
		: public FunctionTraitsImpl<ret_t, arg_pack_t...>
	{};

	// Specialization for member function pointers (const noexcept)
	template <typename class_t, typename ret_t, typename... arg_pack_t>
	struct FunctionTraits<ret_t (class_t::*)(arg_pack_t...) const noexcept>
		: public FunctionTraitsImpl<ret_t, arg_pack_t...>
	{};
	*/
#endif

} // namespace Engine::Detail
