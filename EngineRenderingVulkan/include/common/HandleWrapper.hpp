#pragma once

#include "ImportVulkan.hpp"

#include <type_traits>

namespace Engine::Rendering::Vulkan
{
	template <typename T, bool handle_constructible = false> class HandleWrapper
	{
	public:
		using self_t							  = HandleWrapper<T, handle_constructible>;
		using const_self_t						  = const self_t;
		using value_t							  = T;
		static constexpr bool is_pointer		  = std::is_pointer_v<value_t>;
		static constexpr bool is_bool_convertible = std::is_convertible_v<value_t, bool>;

		HandleWrapper() = default;

		// If handle constructor is allowed
		template <typename conditional_t = int, std::enable_if_t<handle_constructible, conditional_t>* = nullptr>
		HandleWrapper(value_t from_handle) : native_handle(from_handle)
		{
		}

		[[nodiscard]] operator bool() const
		{
			if constexpr (is_pointer)
			{
				return native_handle != VK_NULL_HANDLE;
			}
			else if constexpr (!is_pointer && is_bool_convertible)
			{
				return static_cast<bool>(native_handle);
			}
		}

		[[nodiscard]] value_t& GetHandle()
		{
			return native_handle;
		}

	protected:
		constexpr value_t DefaultVal()
		{
			constexpr bool default_ctor = std::is_default_constructible<value_t>();
			constexpr bool nullptr_ctor = std::is_constructible<value_t, decltype(nullptr)>();

			if constexpr (default_ctor)
			{
				return value_t();
			}
			else if constexpr (nullptr_ctor || is_pointer)
			{
				return nullptr;
			}
		}

		static_assert(!std::is_same<value_t, std::nullptr_t>(), "nullptr_t is not a valid handle type");

		value_t native_handle = DefaultVal();
	};

} // namespace Engine::Rendering::Vulkan
