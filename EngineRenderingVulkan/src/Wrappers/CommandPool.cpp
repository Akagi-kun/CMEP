#include "Wrappers/CommandPool.hpp"

#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/LogicalDevice.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	CommandPool::CommandPool(InstanceOwned::value_t with_instance) : InstanceOwned(with_instance)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType			   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags			   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = logical_device->GetQueueFamilies().graphics_family.value();

		if (vkCreateCommandPool(*logical_device, &pool_info, nullptr, &(native_handle)) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	CommandPool::~CommandPool()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkDestroyCommandPool(*logical_device, native_handle, nullptr);
	}

	[[nodiscard]] CommandBuffer* CommandPool::AllocateCommandBuffer()
	{
		return new CommandBuffer(instance, native_handle);
	}

} // namespace Engine::Rendering::Vulkan
