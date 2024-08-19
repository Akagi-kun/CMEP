#pragma once

#include "HandleWrapper.hpp"
#include "ImportVulkan.hpp"
#include "framework.hpp"

namespace Engine::Rendering::Vulkan
{
	struct MemoryAllocator final : public HandleWrapper<VmaAllocator>
	{
	public:
		MemoryAllocator(Instance* with_instance, vk::Device with_logical_device);
		~MemoryAllocator();
	};
} // namespace Engine::Rendering::Vulkan
