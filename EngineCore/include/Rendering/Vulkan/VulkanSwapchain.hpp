#pragma once

#include "HoldsVulkanDevice.hpp"
#include "vulkan/vulkan_core.h"
namespace Engine::Rendering
{
	class VulkanSwapchain final : public HoldsVulkanDevice
	{
	private:
		VkSwapchainKHR native_handle = VK_NULL_HANDLE;

		std::vector<VkImage> image_handles;
		std::vector<VkImageView> image_view_handles;

		VkFormat image_format{};
		VkExtent2D extent{};

	public:
		VulkanSwapchain(VulkanDeviceManager* with_device_manager, VkExtent2D with_extent, uint32_t with_count);

		~VulkanSwapchain();

		[[nodiscard]] VkSwapchainKHR GetNativeHandle()
		{
			return this->native_handle;
		}

		[[nodiscard]] VkFormat GetImageFormat() const
		{
			return this->image_format;
		}

		[[nodiscard]] std::vector<VkImageView>& GetImageViewHandles()
		{
			return this->image_view_handles;
		}
	};
} // namespace Engine::Rendering
