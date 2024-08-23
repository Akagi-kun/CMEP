#pragma once

#include "ImportVulkan.hpp"
#include "common/StructDefs.hpp"

namespace Engine::Rendering::Vulkan
{
	class Instance;

	class Surface
	{
	public:
		const Instance* created_by;
		vk::SurfaceKHR native_handle;

		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(const vk::raii::PhysicalDevice& device) const;
		[[nodiscard]] bool QueryQueueSupport(const vk::raii::PhysicalDevice& physical_device, uint32_t queue_family)
			const;
	};
} // namespace Engine::Rendering::Vulkan
