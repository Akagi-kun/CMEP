#pragma once

#include <type_traits>

// file containing forward decls

namespace Engine::Rendering::Vulkan
{
	class Instance;
	class PhysicalDevice;
	class LogicalDevice;

	struct DeviceScore;
	struct MemoryAllocator;

	template <typename T, bool handle_constructible>
		requires(!std::is_same_v<T, std::nullptr_t>)
	class HandleWrapper;

	class HoldsVMA;
	class InstanceOwned;

	class Image;
	class ViewedImage;
	template <typename base_t = Image> class SampledImage;

	class Buffer;
	class StagingBuffer;
	class VertexBuffer;
	class UniformBuffer;

	class CommandPool;
	class CommandBuffer;

	class Window;
	class Surface;
	class Swapchain;
	class RenderPass;

	class Pipeline;

	class ShaderModule;
	struct PipelineSettingsN;
} // namespace Engine::Rendering::Vulkan
