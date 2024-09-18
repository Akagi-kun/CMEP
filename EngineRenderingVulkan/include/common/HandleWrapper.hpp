#pragma once

#include <cstddef>
#include <type_traits>

namespace Engine::Rendering::Vulkan
{
	template <typename T, bool handle_constructible = false>
		requires(!std::is_same_v<T, std::nullptr_t>)
	class HandleWrapper
	{
	public:
		using self_t	   = HandleWrapper<T, handle_constructible>;
		using const_self_t = const self_t;
		using value_t	   = T;

		HandleWrapper() = default;

		// If handle constructor is allowed
		template <typename conditional_t = int>
		HandleWrapper(value_t from_handle)
			requires(handle_constructible)
			: native_handle(from_handle)
		{}

		[[nodiscard]] operator bool() const
		{
			if constexpr (is_pointer) { return native_handle != nullptr; }
			else if constexpr (!is_pointer && is_bool_convertible)
			{
				return static_cast<bool>(native_handle);
			}
		}

		[[nodiscard]] value_t& getHandle()
		{
			return native_handle;
		}

	protected:
		static constexpr bool is_pointer		  = std::is_pointer_v<value_t>;
		static constexpr bool is_bool_convertible = std::is_convertible_v<value_t, bool>;
		static constexpr bool default_ctor = std::is_default_constructible_v<value_t>;
		static constexpr bool nullptr_ctor =
			std::is_constructible_v<value_t, std::nullptr_t>;

		constexpr value_t defaultVal()
		{
			if constexpr (default_ctor) { return value_t(); }
			else if constexpr (nullptr_ctor || is_pointer) { return nullptr; }
		}

		value_t native_handle = defaultVal();
	};

} // namespace Engine::Rendering::Vulkan
