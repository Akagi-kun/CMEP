#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"

#include "Rendering/Vulkan/DeviceManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"
#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"
#include "Rendering/Vulkan/Wrappers/Image.hpp"
#include "Rendering/Vulkan/Wrappers/RenderPass.hpp"


namespace Engine::Rendering::Vulkan
{
	Swapchain::Swapchain(DeviceManager* const with_device_manager, VkExtent2D with_extent, uint32_t with_count)
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
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		if (vkCreateSwapchainKHR(
				this->device_manager->GetLogicalDevice(),
				&create_info,
				nullptr,
				&(this->native_handle)
			) != VK_SUCCESS)
		{
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

		VkFormat depth_format = VulkanRenderingEngine::FindVulkanSupportedDepthFormat(
			this->device_manager->GetPhysicalDevice()
		);
		VkFormat color_format = this->GetImageFormat();

		this->depth_buffer = new Image(
			this->device_manager,
			{this->extent.width, this->extent.height},
			this->device_manager->GetMSAASampleCount(),
			depth_format,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		this->depth_buffer->AddImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

		this->multisampled_color_image = new Image(
			this->device_manager,
			{this->extent.width, this->extent.height},
			this->device_manager->GetMSAASampleCount(),
			color_format,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		this->multisampled_color_image->AddImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		this->render_pass = new RenderPass(this->device_manager, this->image_format);

		this->framebuffers.resize(image_view_handles.size());

		for (size_t i = 0; i < image_view_handles.size(); i++)
		{
			std::array<VkImageView, 3> attachments = {
				this->multisampled_color_image->GetNativeViewHandle(),
				this->depth_buffer->GetNativeViewHandle(),
				image_view_handles[i]
			};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass		 = this->render_pass->native_handle;
			framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebuffer_info.pAttachments	 = attachments.data();
			framebuffer_info.width			 = this->extent.width;
			framebuffer_info.height			 = this->extent.height;
			framebuffer_info.layers			 = 1;

			VkDevice logical_device = this->device_manager->GetLogicalDevice();

			if (vkCreateFramebuffer(logical_device, &framebuffer_info, nullptr, &this->framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	Swapchain::~Swapchain()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		for (auto* framebuffer : this->framebuffers)
		{
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}

		for (auto* image_view : this->image_view_handles)
		{
			vkDestroyImageView(logical_device, image_view, nullptr);
		}

		vkDestroySwapchainKHR(logical_device, this->native_handle, nullptr);

		delete multisampled_color_image;
		delete depth_buffer;

		delete render_pass;
	}

	void Swapchain::BeginRenderPass(CommandBuffer* with_buffer, size_t image_index)
	{
		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType			   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass		   = this->render_pass->native_handle;
		render_pass_info.framebuffer	   = this->framebuffers[image_index];
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = this->extent;

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color		 = {{0.0f, 0.0f, 0.0f, 1.0f}}; // TODO: configurable
		clear_values[1].depthStencil = {1.0f, 0};

		render_pass_info.clearValueCount = 2;
		render_pass_info.pClearValues	 = clear_values.data();

		with_buffer->BeginRenderPass(&render_pass_info);
	}

} // namespace Engine::Rendering::Vulkan
