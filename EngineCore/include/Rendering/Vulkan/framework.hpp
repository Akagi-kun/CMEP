#pragma once

// file containing forward decls
// for vulkan wrapper classes

#include <cstdint>

namespace Engine::Rendering::Vulkan
{
	// Base classes
	class HoldsVulkanDevice;
	class HoldsVMA;

	// Command related
	class VCommandPool;
	class VCommandBuffer;

	// Render management
	class VulkanDeviceManager;
	class VulkanRenderingEngine;

	// Image related
	class VImage;
	class VSampledImage;

	// Rendering stack related
	class VulkanSwapchain;

	struct VulkanImageSize
	{
		uint32_t x;
		uint32_t y;
	};
} // namespace Engine::Rendering::Vulkan
