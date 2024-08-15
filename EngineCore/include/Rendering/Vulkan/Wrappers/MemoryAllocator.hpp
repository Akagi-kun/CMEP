#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"

#include "HandleWrapper.hpp"
#include "framework.hpp"

namespace Engine::Rendering::Vulkan
{
	struct MemoryAllocator final : public HandleWrapper<VmaAllocator>
	{
	public:
		MemoryAllocator(Instance* with_instance, VkDevice with_logical_device);
		~MemoryAllocator();
	};
} // namespace Engine::Rendering::Vulkan
