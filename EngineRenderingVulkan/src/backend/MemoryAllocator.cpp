#include "backend/MemoryAllocator.hpp"

#include "backend/Instance.hpp"

namespace Engine::Rendering::Vulkan
{
	MemoryAllocator::MemoryAllocator(Instance* with_instance, vk::Device with_logical_device)
	{
		VmaAllocatorCreateInfo allocator_create_info = {};
		allocator_create_info.vulkanApiVersion		 = vk::ApiVersion11;
		allocator_create_info.physicalDevice		 = **with_instance->GetPhysicalDevice();
		allocator_create_info.device				 = with_logical_device;
		allocator_create_info.instance				 = *with_instance->GetHandle();

		vmaCreateAllocator(&allocator_create_info, &native_handle);
	}

	MemoryAllocator::~MemoryAllocator()
	{
		vmaDestroyAllocator(native_handle);
	}
} // namespace Engine::Rendering::Vulkan
