#include "Rendering/Vulkan/VSwapchain.hpp"

#include "Rendering/Vulkan/HoldsVulkanDevice.hpp"
#include "Rendering/Vulkan/VulkanDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"

namespace Engine::Rendering::Vulkan
{
	VSwapchain::VSwapchain(VulkanDeviceManager* const with_device_manager, VkExtent2D with_extent, uint32_t with_count)
		: HoldsVulkanDevice(with_device_manager), image_format(VK_FORMAT_B8G8R8A8_UNORM), extent(with_extent)
	{
		// Query details for support of swapchains
		SwapChainSupportDetails swap_chain_support = this->device_manager->QuerySwapChainSupport();
		VkSurfaceFormatKHR surface_format = Vulkan::Utils::ChooseVulkanSwapSurfaceFormat(swap_chain_support.formats);

		VkPresentModeKHR present_mode = Vulkan::Utils::ChooseVulkanSwapPresentMode(swap_chain_support.present_modes);

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType			 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface			 = this->device_manager->GetSurface();
		create_info.minImageCount	 = with_count;
		create_info.imageFormat		 = VK_FORMAT_B8G8R8A8_UNORM;
		create_info.imageColorSpace	 = surface_format.colorSpace;
		create_info.imageExtent		 = with_extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage		 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.preTransform	 = swap_chain_support.capabilities.currentTransform;
		create_info.compositeAlpha	 = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode		 = present_mode;
		create_info.clipped			 = VK_TRUE;

		create_info.oldSwapchain = VK_NULL_HANDLE;

		QueueFamilyIndices queue_indices = this->device_manager->GetQueueFamilies();
		uint32_t queue_family_indices[] = {queue_indices.graphics_family.value(), queue_indices.present_family.value()};

		if (queue_indices.graphics_family != queue_indices.present_family)
		{
			create_info.imageSharingMode	  = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices	  = queue_family_indices;

			// this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Using concurrent sharing mode");
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			// this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Using exclusive sharing mode");
		}

		if (vkCreateSwapchainKHR(
				this->device_manager->GetLogicalDevice(),
				&create_info,
				nullptr,
				&(this->native_handle)
			) != VK_SUCCESS)
		{
			// this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan swap chain creation failed");
			throw std::runtime_error("Failed to create swap chain!");
		}

		uint32_t swapchain_image_count = 0;

		// Get image count
		vkGetSwapchainImagesKHR(
			this->device_manager->GetLogicalDevice(),
			this->native_handle,
			&swapchain_image_count,
			nullptr
		);

		// Get images proper
		this->image_handles.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(
			this->device_manager->GetLogicalDevice(),
			this->native_handle,
			&swapchain_image_count,
			this->image_handles.data()
		);

		// Create image views
		this->image_view_handles.resize(this->image_handles.size());
		for (size_t i = 0; i < this->image_handles.size(); i++)
		{
			VkImageViewCreateInfo view_info{};
			view_info.sType							  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image							  = this->image_handles[i];
			view_info.viewType						  = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format						  = VK_FORMAT_B8G8R8A8_UNORM;
			view_info.subresourceRange.aspectMask	  = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel	  = 0;
			view_info.subresourceRange.levelCount	  = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount	  = 1;

			if (vkCreateImageView(
					this->device_manager->GetLogicalDevice(),
					&view_info,
					nullptr,
					&this->image_view_handles[i]
				) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swapchain image view!");
			}
		}
	}

	VSwapchain::~VSwapchain()
	{
		for (auto* image_view : this->image_view_handles)
		{
			vkDestroyImageView(this->device_manager->GetLogicalDevice(), image_view, nullptr);
		}

		vkDestroySwapchainKHR(this->device_manager->GetLogicalDevice(), this->native_handle, nullptr);
	}
} // namespace Engine::Rendering::Vulkan
