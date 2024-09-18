#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "common/StructDefs.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>

namespace Engine::Rendering::Vulkan
{
	class Instance;

	class Surface
	{
	public:
		const Instance* created_by;
		vk::SurfaceKHR	native_handle;

		[[nodiscard]] SwapChainSupportDetails
		querySwapChainSupport(const vk::raii::PhysicalDevice& device) const;

		[[nodiscard]] bool queryQueueSupport(
			const vk::raii::PhysicalDevice& physical_device,
			uint32_t						queue_family
		) const;
	};
} // namespace Engine::Rendering::Vulkan
