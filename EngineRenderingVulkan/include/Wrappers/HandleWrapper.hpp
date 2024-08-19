#pragma once

#include "ImportVulkan.hpp"

#include <stdexcept>
#include <type_traits>

namespace Engine::Rendering::Vulkan
{
	template <typename T, bool handle_constructible = false> class HandleWrapper
	{
	public:
		using self_t	   = HandleWrapper<T, handle_constructible>;
		using const_self_t = const self_t;

		using value_t  = T;
		using is_ptr_t = std::is_pointer<value_t>;

		HandleWrapper() = default;

		// If handle constructor is allowed
		template <
			typename conditional_enable_t								  = int,
			std::enable_if_t<handle_constructible, conditional_enable_t>* = nullptr>
		HandleWrapper(value_t from_handle) : native_handle(from_handle)
		{
		}

		// If handle is a pointer
		template <
			typename conditional_enable_t							 = int,
			std::enable_if_t<is_ptr_t::value, conditional_enable_t>* = nullptr>
		[[nodiscard]] operator bool() const
		{
			return native_handle != VK_NULL_HANDLE;
		}

		// If handle isn't a pointer
		template <
			typename conditional_enable_t							  = int,
			std::enable_if_t<!is_ptr_t::value, conditional_enable_t>* = nullptr>
		[[nodiscard]] operator bool() const
		{
			return static_cast<bool>(native_handle);
		}

		[[nodiscard]] operator value_t()
		{
			// bool conversion op is const and so will only be called on const objects
			if (!static_cast<bool>(static_cast<const_self_t>(*this)))
			{
				throw std::runtime_error("Tried to acquire handle of an empty HandleWrapper!");
			}

			return native_handle;
		}

		[[nodiscard]] value_t GetHandle()
		{
			return native_handle;
		}

	protected:
		static_assert(!std::is_same<value_t, std::nullptr_t>(), "nullptr_t is not a valid handle type");

		value_t native_handle{};
	};

} // namespace Engine::Rendering::Vulkan
