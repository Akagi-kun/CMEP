#include "objects/CommandPool.hpp"

#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"
#include "objects/CommandBuffer.hpp"

namespace Engine::Rendering::Vulkan
{
	CommandPool::CommandPool(InstanceOwned::value_t with_instance) : InstanceOwned(with_instance)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::CommandPoolCreateInfo pool_info(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			logical_device->GetQueueFamilies().graphics_family
		);

		native_handle = logical_device->createCommandPool(pool_info);
	}

	[[nodiscard]] CommandBuffer* CommandPool::AllocateCommandBuffer()
	{
		return new CommandBuffer(instance->GetLogicalDevice(), native_handle);
	}

	[[nodiscard]] CommandBuffer CommandPool::ConstructCommandBuffer()
	{
		return {instance->GetLogicalDevice(), native_handle};
	}

} // namespace Engine::Rendering::Vulkan
