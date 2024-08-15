#pragma once

#include "HandleWrapper.hpp"
#include "InstanceOwned.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class CommandPool : public InstanceOwned, public HandleWrapper<VkCommandPool>
	{
	public:
		CommandPool(InstanceOwned::value_t with_instance);

		~CommandPool();

		[[nodiscard]] CommandBuffer* AllocateCommandBuffer();
	};
} // namespace Engine::Rendering::Vulkan
