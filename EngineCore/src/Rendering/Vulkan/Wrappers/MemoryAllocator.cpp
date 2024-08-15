#include "Rendering/Vulkan/Wrappers/MemoryAllocator.hpp"

#include "Rendering/Vulkan/Wrappers/Instance.hpp"

namespace Engine::Rendering::Vulkan
{
	MemoryAllocator::MemoryAllocator(Instance* with_instance, VkDevice with_logical_device)
	{
		VmaAllocatorCreateInfo allocator_create_info = {};
		allocator_create_info.vulkanApiVersion		 = VK_API_VERSION_1_1; // NOLINT
		allocator_create_info.physicalDevice		 = with_instance->GetPhysicalDevice();
		allocator_create_info.device				 = with_logical_device;
		allocator_create_info.instance				 = *with_instance;

		vmaCreateAllocator(&allocator_create_info, &(native_handle));
	}

	MemoryAllocator::~MemoryAllocator()
	{
		vmaDestroyAllocator(native_handle);
	}
} // namespace Engine::Rendering::Vulkan
