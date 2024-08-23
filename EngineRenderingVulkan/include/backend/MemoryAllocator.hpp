#pragma once

#include "fwd.hpp"

#include "ImportVulkan.hpp"
#include "common/HandleWrapper.hpp"

// Include VMA
#include "vk_mem_alloc.h" // IWYU pragma: export

namespace Engine::Rendering::Vulkan
{
	struct MemoryAllocator final : public HandleWrapper<VmaAllocator>
	{
	public:
		MemoryAllocator(Instance* with_instance, vk::raii::Device& with_device);
		~MemoryAllocator();
	};
} // namespace Engine::Rendering::Vulkan
