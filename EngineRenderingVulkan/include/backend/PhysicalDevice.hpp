#pragma once
// IWYU pragma: private; include Rendering/Vulkan/backend.hpp

#include "common/StructDefs.hpp"
#include "rendering/Surface.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class PhysicalDevice final : public vk::raii::PhysicalDevice
	{
	public:
		PhysicalDevice(vk::raii::PhysicalDevice from_device)
			: vk::raii::PhysicalDevice(std::move(from_device))
		{
		}

		[[nodiscard]] vk::PhysicalDeviceLimits getLimits() const;
		[[nodiscard]] vk::SampleCountFlagBits  getMSAASamples() const;

		[[nodiscard]] std::string getDeviceName() const;

		[[nodiscard]] vk::Format findSupportedFormat(
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling				   tiling,
			vk::FormatFeatureFlags		   features
		) const;

		[[nodiscard]] vk::Format findSupportedDepthFormat() const;

		[[nodiscard]] std::optional<QueueFamilyIndices> findVulkanQueueFamilies(const Surface* with_surface
		) const;
	};

} // namespace Engine::Rendering::Vulkan
