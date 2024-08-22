#pragma once

#include "ImportVulkan.hpp"
#include "common/StructDefs.hpp"
#include "rendering/Surface.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class PhysicalDevice final : public vk::raii::PhysicalDevice
	{
	public:
		PhysicalDevice(vk::raii::PhysicalDevice from_device) : vk::raii::PhysicalDevice(std::move(from_device))
		{
		}

		[[nodiscard]] std::string GetDeviceName() const;

		[[nodiscard]] vk::Format FindSupportedFormat(
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling,
			vk::FormatFeatureFlags features
		) const;

		[[nodiscard]] vk::Format FindSupportedDepthFormat() const;

		[[nodiscard]] std::optional<QueueFamilyIndices> FindVulkanQueueFamilies(const Surface* with_surface) const;
	};

} // namespace Engine::Rendering::Vulkan
