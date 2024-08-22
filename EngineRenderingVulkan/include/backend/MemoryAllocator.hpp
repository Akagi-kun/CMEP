#pragma once

#include "ImportVulkan.hpp"
#include "common/HandleWrapper.hpp"
#include "fwd.hpp"

// Include VMA
#include "vk_mem_alloc.h" // IWYU pragma: export

namespace Engine::Rendering::Vulkan
{
	struct MemoryAllocator final : public HandleWrapper<VmaAllocator>
	{
	public:
		MemoryAllocator(Instance* with_instance, vk::Device with_logical_device);
		~MemoryAllocator();
	};
} // namespace Engine::Rendering::Vulkan
