#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "vulkan/vulkan_core.h"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

	////////////////////////////////////////////////////////////////////////
	////////////////////////    Init functions    //////////////////////////
	////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::CreateVulkanSwapChain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swap_chain_support = this->device_manager->QuerySwapChainSupport();

		// Get the info out of the capabilities
		VkSurfaceFormatKHR surface_format = VulkanUtils::ChooseVulkanSwapSurfaceFormat(swap_chain_support.formats);
		VkPresentModeKHR present_mode	  = VulkanUtils::ChooseVulkanSwapPresentMode(swap_chain_support.present_modes);
		VkExtent2D extent				  = this->ChooseVulkanSwapExtent(swap_chain_support.capabilities);

		if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug1,
				LOGPFX_CURRENT "Present mode is VK_PRESENT_MODE_MAILBOX_KHR"
			);
		}
		else if (present_mode == VK_PRESENT_MODE_FIFO_KHR)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug1,
				LOGPFX_CURRENT "Present mode is VK_PRESENT_MODE_FIFO_KHR"
			);
		}
		else
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Present mode is other");
		}

		// Request one image more than is the required minimum
		uint32_t swapchain_image_count = swap_chain_support.capabilities.minImageCount + 1;

		// Check if there is a defined maximum (maxImageCount > 0)
		// where 0 is a special value meaning no maximum
		//
		// And if there is a maximum, clamp swap chain length to it
		if (swap_chain_support.capabilities.maxImageCount > 0 &&
			swapchain_image_count > swap_chain_support.capabilities.maxImageCount)
		{
			swapchain_image_count = swap_chain_support.capabilities.maxImageCount;
			this->logger->SimpleLog(
				Logging::LogLevel::Debug1,
				LOGPFX_CURRENT "Using maxImageCount capability, GPU support limited"
			);
		}

		// Save this value to be used later
		// this->max_frames_in_flight = swapchain_image_count;

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = this->device_manager->GetSurface();

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Creating Vulkan swap chain with %u images (max supported: %u)",
			swapchain_image_count,
			swap_chain_support.capabilities.maxImageCount
		);

		create_info.minImageCount	 = swapchain_image_count;
		create_info.imageFormat		 = VK_FORMAT_B8G8R8A8_UNORM;
		create_info.imageColorSpace	 = surface_format.colorSpace;
		create_info.imageExtent		 = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage		 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

		create_info.preTransform   = swap_chain_support.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode	   = present_mode;
		create_info.clipped		   = VK_TRUE;

		create_info.oldSwapchain = VK_NULL_HANDLE;

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &(this->vk_swap_chain)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan swap chain creation failed");
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(logical_device, this->vk_swap_chain, &swapchain_image_count, nullptr);
		this->vk_swap_chain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(
			logical_device,
			this->vk_swap_chain,
			&swapchain_image_count,
			this->vk_swap_chain_images.data()
		);

		this->vk_swap_chain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
		this->vk_swap_chain_extent		 = extent;

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Vulkan swap chain created");
	}

	void VulkanRenderingEngine::RecreateVulkanSwapChain()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		// If window is minimized, wait for it to show up again
		int width  = 0;
		int height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logical_device);

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Recreating vulkan swap chain");

		// Clean up old swap chain
		this->CleanupVulkanSwapChain();
		this->CleanupVulkanImage(this->vk_depth_buffer);
		this->CleanupVulkanImage(this->multisampled_color_image);

		// Create a new swap chain
		this->CreateVulkanSwapChain();
		this->CreateVulkanSwapChainViews();
		this->CreateVulkanDepthResources();
		this->CreateMultisampledColorResources();
		this->CreateVulkanFramebuffers();
	}

	void VulkanRenderingEngine::CleanupVulkanSwapChain()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		for (auto* framebuffer : this->vk_swap_chain_framebuffers)
		{
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}

		for (auto* image_view : this->vk_swap_chain_image_views)
		{
			vkDestroyImageView(logical_device, image_view, nullptr);
		}

		vkDestroySwapchainKHR(logical_device, this->vk_swap_chain, nullptr);
	}

	void VulkanRenderingEngine::CreateVulkanSwapChainViews()
	{
		this->vk_swap_chain_image_views.resize(this->vk_swap_chain_images.size());

		if (const auto& vulkan_image_factory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			for (size_t i = 0; i < this->vk_swap_chain_images.size(); i++)
			{
				this->vk_swap_chain_image_views[i] = vulkan_image_factory->CreateImageView(
					this->vk_swap_chain_images[i],
					VK_FORMAT_B8G8R8A8_UNORM,
					VK_IMAGE_ASPECT_COLOR_BIT
				);
			}
		}
	}

	VkShaderModule VulkanRenderingEngine::CreateVulkanShaderModule(const std::vector<char>& code)
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		VkShaderModuleCreateInfo create_info{};
		create_info.sType	 = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = code.size();
		create_info.pCode	 = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shader_module;
		if (vkCreateShaderModule(logical_device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating shader module");
			throw std::runtime_error("failed to create shader module!");
		}

		return shader_module;
	}

	void VulkanRenderingEngine::CreateVulkanDefaultGraphicsPipeline()
	{
		VulkanPipelineSettings pipeline_settings  = this->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);

		this->graphics_pipeline_default = this->CreateVulkanPipeline(
			pipeline_settings,
			"game/shaders/vulkan/default_vert.spv",
			"game/shaders/vulkan/default_frag.spv"
		);
	}

	void VulkanRenderingEngine::CreateVulkanRenderPass()
	{
		VkAttachmentDescription color_attachment{};
		color_attachment.format			= this->vk_swap_chain_image_format;
		color_attachment.samples		= this->device_manager->GetMSAASampleCount();
		color_attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment{};
		depth_attachment.format			= this->FindVulkanSupportedDepthFormat();
		depth_attachment.samples		= this->device_manager->GetMSAASampleCount();
		depth_attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription color_attachment_resolve{};
		color_attachment_resolve.format			= this->vk_swap_chain_image_format;
		color_attachment_resolve.samples		= VK_SAMPLE_COUNT_1_BIT;
		color_attachment_resolve.loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_resolve.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_resolve.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_resolve.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_resolve.finalLayout	= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref{};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment_resolve_ref{};
		color_attachment_resolve_ref.attachment = 2;
		color_attachment_resolve_ref.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= 1;
		subpass.pColorAttachments		= &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;
		subpass.pResolveAttachments		= &color_attachment_resolve_ref;

		std::array<VkAttachmentDescription, 3> attachments =
			{color_attachment, depth_attachment, color_attachment_resolve};

		VkSubpassDependency dependency{};
		dependency.srcSubpass	= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass	= 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
								  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
								  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info{};
		render_pass_info.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments	 = attachments.data();
		render_pass_info.subpassCount	 = 1;
		render_pass_info.pSubpasses		 = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies	 = &dependency;

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		if (vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &this->vk_render_pass) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating render pass");
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanRenderingEngine::CreateVulkanFramebuffers()
	{
		this->vk_swap_chain_framebuffers.resize(this->vk_swap_chain_image_views.size());

		for (size_t i = 0; i < vk_swap_chain_image_views.size(); i++)
		{
			std::array<VkImageView, 3> attachments = {
				this->multisampled_color_image->image_view,
				this->vk_depth_buffer->image_view,
				this->vk_swap_chain_image_views[i]
			};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass		 = this->vk_render_pass;
			framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebuffer_info.pAttachments	 = attachments.data();
			framebuffer_info.width			 = this->vk_swap_chain_extent.width;
			framebuffer_info.height			 = this->vk_swap_chain_extent.height;
			framebuffer_info.layers			 = 1;

			VkDevice logical_device = this->device_manager->GetLogicalDevice();

			if (vkCreateFramebuffer(logical_device, &framebuffer_info, nullptr, &this->vk_swap_chain_framebuffers[i]) !=
				VK_SUCCESS)
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating framebuffers");
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VulkanRenderingEngine::CreateVulkanCommandPools()
	{
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType			   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags			   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = this->device_manager->GetQueueFamilies().graphics_family.value();

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		if (vkCreateCommandPool(logical_device, &pool_info, nullptr, &(this->vk_command_pool)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating command pools");
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void VulkanRenderingEngine::CreateVulkanCommandBuffers()
	{
		vk_command_buffers.resize(this->max_frames_in_flight);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool		  = this->vk_command_pool;
		alloc_info.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = static_cast<uint32_t>(vk_command_buffers.size());

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		if (vkAllocateCommandBuffers(logical_device, &alloc_info, this->vk_command_buffers.data()) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating command pools");
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VulkanRenderingEngine::CreateVulkanSyncObjects()
	{
		this->image_available_semaphores.resize(this->max_frames_in_flight);
		this->render_finished_semaphores.resize(this->max_frames_in_flight);
		this->in_flight_fences.resize(this->max_frames_in_flight);

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < this->max_frames_in_flight; i++)
		{
			VkDevice logical_device = this->device_manager->GetLogicalDevice();

			if (vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &(this->image_available_semaphores[i])) !=
					VK_SUCCESS ||
				vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &(this->render_finished_semaphores[i])) !=
					VK_SUCCESS ||
				vkCreateFence(logical_device, &fence_info, nullptr, &(this->in_flight_fences[i])) != VK_SUCCESS)
			{

				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating sync objects");
				throw std::runtime_error("failed to create sync objects!");
			}
		}
	}

	void VulkanRenderingEngine::CreateVulkanDepthResources()
	{
		VkFormat depth_format = this->FindVulkanSupportedDepthFormat();

		if (const auto& vulkan_image_factory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			this->vk_depth_buffer = vulkan_image_factory->CreateImage(
				this->vk_swap_chain_extent.width,
				this->vk_swap_chain_extent.height,
				this->device_manager->GetMSAASampleCount(),
				depth_format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

			this->vk_depth_buffer->image_view = vulkan_image_factory->CreateImageView(
				this->vk_depth_buffer->image,
				depth_format,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
		}
	}

	void VulkanRenderingEngine::CreateMultisampledColorResources()
	{
		VkFormat color_format = this->vk_swap_chain_image_format;

		if (const auto& vulkan_image_factory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			this->multisampled_color_image = vulkan_image_factory->CreateImage(
				this->vk_swap_chain_extent.width,
				this->vk_swap_chain_extent.height,
				this->device_manager->GetMSAASampleCount(),
				color_format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

			this->multisampled_color_image->image_view = vulkan_image_factory->CreateImageView(
				this->multisampled_color_image->image,
				this->multisampled_color_image->image_format,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
		}
	}

	void VulkanRenderingEngine::CreateVulkanMemoryAllocator()
	{
		VmaAllocatorCreateInfo allocator_create_info = {};
		allocator_create_info.vulkanApiVersion		 = VK_API_VERSION_1_1;
		allocator_create_info.physicalDevice		 = this->device_manager->GetPhysicalDevice();
		allocator_create_info.device				 = this->device_manager->GetLogicalDevice();
		allocator_create_info.instance				 = this->device_manager->GetInstance();

		vmaCreateAllocator(&allocator_create_info, &(this->vma_allocator));

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "VMA created");
	}
} // namespace Engine::Rendering
