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
	class VDeviceManager;
	class VulkanRenderingEngine;

	// Image related
	class VImage;
	class VSampledImage;
	struct VImageSize
	{
		uint32_t x;
		uint32_t y;
	};

	// Rendering stack related
	class VSwapchain;
	class VPipeline;

	// Buffer related
	class VBuffer;
} // namespace Engine::Rendering::Vulkan
