#pragma once

// file containing forward decls
// for vulkan wrapper classes

#include <cstdint>

namespace Engine::Rendering::Vulkan
{
	// Base classes
	class InstanceOwned;
	class HoldsVMA;

	// Command related
	class CommandPool;
	class CommandBuffer;

	// Render management
	class Instance;
	class PhysicalDevice;
	class LogicalDevice;
	class Queue;

	class DeviceManager;
	class VulkanRenderingEngine;

	// Image related
	class Image;
	class SampledImage;
	/* struct ImageSize // TODO: Replace with Vector2
	{
		uint32_t x;
		uint32_t y;
	}; */

	// Rendering stack related
	class Swapchain;
	class Pipeline;
	struct RenderPass;
	struct Surface;

	// Buffer related
	class Buffer;
} // namespace Engine::Rendering::Vulkan
