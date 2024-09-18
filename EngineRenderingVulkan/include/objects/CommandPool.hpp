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

		[[nodiscard]] CommandBuffer* allocateCommandBuffer();
		[[nodiscard]] CommandBuffer	 constructCommandBuffer();
	};
} // namespace Engine::Rendering::Vulkan
