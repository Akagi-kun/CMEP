#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"

#include "Rendering/Vulkan/VulkanUtilities.hpp"
#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/CommandPool.hpp"
#include "Rendering/Vulkan/Wrappers/Image.hpp"
#include "Rendering/Vulkan/Wrappers/Instance.hpp"
#include "Rendering/Vulkan/Wrappers/RenderPass.hpp"

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Swapchain::Swapchain(
		InstanceOwned::value_t with_instance,
		Surface* with_surface,
		VkExtent2D with_extent,
		uint32_t with_count
	)
		: InstanceOwned(with_instance), image_format(VK_FORMAT_B8G8R8A8_UNORM), extent(with_extent)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		// Query details for support of swapchains
		SwapChainSupportDetails swap_chain_support = with_surface->QueryVulkanSwapChainSupport(
			instance->GetPhysicalDevice()
		);
		VkSurfaceFormatKHR surface_format = Vulkan::Utils::ChooseVulkanSwapSurfaceFormat(swap_chain_support.formats);

		VkPresentModeKHR present_mode = Vulkan::Utils::ChooseVulkanSwapPresentMode(swap_chain_support.present_modes);

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType			 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface			 = with_surface->native_handle; // this->device_manager->GetSurface();
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

		QueueFamilyIndices queue_indices = logical_device->GetQueueFamilies();
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

		if (vkCreateSwapchainKHR(*logical_device, &create_info, nullptr, &(this->native_handle)) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain!");
		}

		// TODO: Use max_frames_in_flight instead and only assert they're ==
		uint32_t swapchain_image_count = 0;

		// Get image count
		vkGetSwapchainImagesKHR(*logical_device, this->native_handle, &swapchain_image_count, nullptr);

		// Get images proper
		this->image_handles.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(
			*logical_device,
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

			if (vkCreateImageView(*logical_device, &view_info, nullptr, &this->image_view_handles[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swapchain image view!");
			}
		}

		VkFormat depth_format = instance->GetPhysicalDevice().FindSupportedDepthFormat();
		VkFormat color_format = this->GetImageFormat();

		this->depth_buffer = new Image(
			instance,
			{this->extent.width, this->extent.height},
			instance->GetMSAASamples(),
			depth_format,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
		this->depth_buffer->AddImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

		this->multisampled_color_image = new Image(
			instance,
			{this->extent.width, this->extent.height},
			instance->GetMSAASamples(),
			color_format,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		);
		this->multisampled_color_image->AddImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		this->render_pass = new RenderPass(instance, this->image_format);

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

			if (vkCreateFramebuffer(*logical_device, &framebuffer_info, nullptr, &this->framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		for (auto& target : this->render_targets)
		{
			this->CreateRenderTarget(target);
		}
	}

	Swapchain::~Swapchain()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		for (auto* framebuffer : this->framebuffers)
		{
			vkDestroyFramebuffer(*logical_device, framebuffer, nullptr);
		}

		for (auto* image_view : this->image_view_handles)
		{
			vkDestroyImageView(*logical_device, image_view, nullptr);
		}

		vkDestroySwapchainKHR(*logical_device, this->native_handle, nullptr);

		delete multisampled_color_image;
		delete depth_buffer;

		delete render_pass;

		for (auto& target : this->render_targets)
		{
			this->CleanupRenderTarget(target);
		}
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

	void Swapchain::RenderFrame(
		CommandBuffer* command_buffer,
		uint32_t image_index,
		const std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)>& callback,
		void* user_data
	)
	{
		command_buffer->BeginCmdBuffer(0);

		BeginRenderPass(command_buffer, image_index);

		VkViewport viewport{};
		viewport.x		  = 0.0f;
		viewport.y		  = 0.0f;
		viewport.width	  = static_cast<float>(extent.width);
		viewport.height	  = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(*command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = extent;
		vkCmdSetScissor(*command_buffer, 0, 1, &scissor);

		// Perform actual render
		if (!callback)
		{
			throw std::invalid_argument("Tried to perform frame render without a callback!");
		}

		assert(callback);
		callback(command_buffer, image_index, user_data);

		command_buffer->EndRenderPass();
		command_buffer->EndCmdBuffer();
	}

#pragma region Private

	void Swapchain::CreateRenderTarget(RenderTargetData& target)
	{
		target.command_buffer = instance->GetCommandPool()->AllocateCommandBuffer();

		this->CreateSyncObjects(target.sync_objects);
	}

	void Swapchain::CreateSyncObjects(SyncObjects& sync_objects)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		static VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		static VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(*logical_device, &semaphore_info, nullptr, &(sync_objects.image_available)) !=
				VK_SUCCESS ||
			vkCreateSemaphore(*logical_device, &semaphore_info, nullptr, &(sync_objects.present_ready)) != VK_SUCCESS ||
			vkCreateFence(*logical_device, &fence_info, nullptr, &(sync_objects.in_flight)) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create sync objects!");
		}
	}

	void Swapchain::CleanupRenderTarget(RenderTargetData& target)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkDestroySemaphore(*logical_device, target.sync_objects.present_ready, nullptr);
		vkDestroySemaphore(*logical_device, target.sync_objects.image_available, nullptr);
		vkDestroyFence(*logical_device, target.sync_objects.in_flight, nullptr);

		delete target.command_buffer;
	}
} // namespace Engine::Rendering::Vulkan
