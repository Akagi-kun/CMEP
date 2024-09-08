#include "objects/CommandPool.hpp"

#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"
#include "objects/CommandBuffer.hpp"

namespace Engine::Rendering::Vulkan
{
	CommandPool::CommandPool(InstanceOwned::value_t with_instance)
		: InstanceOwned(with_instance)
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		vk::CommandPoolCreateInfo pool_info(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			logical_device->getQueueFamilies().graphics_family
		);

		native_handle = logical_device->createCommandPool(pool_info);
	}

	[[nodiscard]] CommandBuffer* CommandPool::allocateCommandBuffer()
	{
		return new CommandBuffer(instance->getLogicalDevice(), native_handle);
	}

	[[nodiscard]] CommandBuffer CommandPool::constructCommandBuffer()
	{
		return {instance->getLogicalDevice(), native_handle};
	}

} // namespace Engine::Rendering::Vulkan
