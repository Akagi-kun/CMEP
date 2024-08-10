#pragma once

#include "CommandBuffer.hpp"
#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Swapchain final : public HoldsVulkanDevice
	{
	private:
		VkSwapchainKHR native_handle = VK_NULL_HANDLE;

		std::vector<VkImage> image_handles;
		std::vector<VkImageView> image_view_handles;

		VkFormat image_format{};
		const VkExtent2D extent;

		RenderPass* render_pass;

		std::vector<VkFramebuffer> framebuffers;

		// Multisampling
		Image* multisampled_color_image = nullptr;
		// Depth buffers
		Image* depth_buffer				= nullptr;

	public:
		Swapchain(DeviceManager* with_device_manager, VkExtent2D with_extent, uint32_t with_count);
		~Swapchain();

		void BeginRenderPass(CommandBuffer* with_buffer, size_t image_index);

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

		[[nodiscard]] const VkExtent2D& GetExtent() const
		{
			return this->extent;
		}

		[[nodiscard]] VkFramebuffer GetFramebuffer(size_t idx)
		{
			return this->framebuffers[idx];
		}

		[[nodiscard]] RenderPass* GetRenderPass()
		{
			return this->render_pass;
		}
	};
} // namespace Engine::Rendering::Vulkan
