#pragma once

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "common/InstanceOwned.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class CommandPool : public InstanceOwned, public HandleWrapper<vk::raii::CommandPool>
	{
	public:
		CommandPool(InstanceOwned::value_t with_instance);

		~CommandPool() = default;

		/**
		 * Allocate and construct a @ref CommandBuffer in memory
		 *
		 * @return A pointer to the allocated command buffer
		 */
		[[nodiscard]] CommandBuffer* allocateCommandBuffer();

		/**
		 * Construct a @ref CommandBuffer and return it by value
		 *
		 * @return The command buffer
		 */
		[[nodiscard]] CommandBuffer constructCommandBuffer();
	};
} // namespace Engine::Rendering::Vulkan
