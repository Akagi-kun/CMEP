#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/VulkanDeviceManager.hpp"

#include <memory>

namespace Engine::Rendering
{
	class VulkanManagedBase
	{
	protected:
		std::shared_ptr<VulkanDeviceManager> device_manager;
		VmaAllocator allocator = nullptr;

	public:
		VulkanManagedBase() = delete;
		VulkanManagedBase(const std::shared_ptr<VulkanDeviceManager>& with_device_manager, VmaAllocator with_allocator)
			: device_manager(with_device_manager), allocator(with_allocator)
		{
		}
	};
} // namespace Engine::Rendering
