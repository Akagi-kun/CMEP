#include "Wrappers/CommandPool.hpp"

#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/LogicalDevice.hpp"

namespace Engine::Rendering::Vulkan
{
	CommandPool::CommandPool(InstanceOwned::value_t with_instance) : InstanceOwned(with_instance)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::CommandPoolCreateInfo pool_info(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			logical_device->GetQueueFamilies().graphics_family.value()
		);

		native_handle = logical_device->GetHandle().createCommandPool(pool_info);
	}

	CommandPool::~CommandPool()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().destroyCommandPool(native_handle);
	}

	[[nodiscard]] CommandBuffer* CommandPool::AllocateCommandBuffer()
	{
		return new CommandBuffer(instance, native_handle);
	}

	[[nodiscard]] CommandBuffer CommandPool::AllocateTemporaryCommandBuffer()
	{
		return {instance, native_handle};
	}

} // namespace Engine::Rendering::Vulkan
