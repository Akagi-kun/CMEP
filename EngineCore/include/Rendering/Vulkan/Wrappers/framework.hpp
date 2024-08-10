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
	class CommandPool;
	class CommandBuffer;

	// Render management
	class DeviceManager;
	class VulkanRenderingEngine;

	// Image related
	class Image;
	class SampledImage;
	struct ImageSize
	{
		uint32_t x;
		uint32_t y;
	};

	// Rendering stack related
	class Swapchain;
	class Pipeline;
	class RenderPass;

	// Buffer related
	class Buffer;
} // namespace Engine::Rendering::Vulkan
