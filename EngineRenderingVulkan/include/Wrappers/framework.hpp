#pragma once

// file containing forward decls
// for vulkan wrapper classes

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

	class DeviceManager;
	class VulkanRenderingEngine;

	// Image related
	class Image;
	class SampledImage;

	// Rendering stack related
	class Swapchain;
	class Pipeline;
	struct RenderPass;
	struct Surface;

	// Buffer related
	class Buffer;
} // namespace Engine::Rendering::Vulkan
