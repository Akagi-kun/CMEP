#pragma once

#include "HandleWrapper.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "framework.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class CommandPool : public InstanceOwned, public HandleWrapper<vk::raii::CommandPool>
	{
	public:
		CommandPool(InstanceOwned::value_t with_instance);

		~CommandPool() = default;

		[[nodiscard]] CommandBuffer* AllocateCommandBuffer();
		[[nodiscard]] CommandBuffer AllocateTemporaryCommandBuffer();
	};
} // namespace Engine::Rendering::Vulkan
