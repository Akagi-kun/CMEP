#pragma once

#include "vulkan/vulkan_core.h"

#include <stdexcept>
#include <type_traits>

namespace Engine::Rendering::Vulkan
{
	template <typename T, bool handle_constructible = false> class HandleWrapper
	{
	public:
		using self_t  = HandleWrapper<T>;
		using value_t = T;

		HandleWrapper() = default;

		// Use handle_constructible = true to enable this object to be constructed from its handle representation
		template <
			typename conditional_enable_t								  = int,
			std::enable_if_t<handle_constructible, conditional_enable_t>* = nullptr>
		HandleWrapper(value_t from_handle) : native_handle(from_handle)
		{
		}

		operator bool() const
		{
			return native_handle != VK_NULL_HANDLE;
		}

		operator value_t() const
		{
			if (!static_cast<bool>(*this))
			{
				throw std::runtime_error("Tried to acquire handle of an empty HandleWrapper!");
			}

			return native_handle;
		}

	protected:
		static_assert(std::is_pointer<value_t>(), "HandleWrapper can only wrap pointer types");
		static_assert(!std::is_same<value_t, nullptr_t>(), "nullptr_t is not a valid handle type");

		value_t native_handle = VK_NULL_HANDLE;
	};

} // namespace Engine::Rendering::Vulkan
