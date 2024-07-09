#include "Rendering/Vulkan/VImage.hpp"
#include "Rendering/Vulkan/VSwapchain.hpp"
#include "Rendering/Vulkan/VulkanDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "Logging/Logging.hpp"

#include "vulkan/vulkan_core.h"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering::Vulkan
{
	////////////////////////////////////////////////////////////////////////
	////////////////////////    Init functions    //////////////////////////
	////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::CreateVulkanSwapChain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swap_chain_support = this->device_manager->QuerySwapChainSupport();

		VkExtent2D extent = this->ChooseVulkanSwapExtent(swap_chain_support.capabilities);

		// Request one image more than is the required minimum
		// uint32_t swapchain_image_count = swap_chain_support.capabilities.minImageCount + 1;
		// Temporary fix for screen lag
		// uint32_t swapchain_image_count = 1;
		uint32_t swapchain_image_count = 3;

		// Check if there is a defined maximum (maxImageCount > 0)
		// where 0 is a special value meaning no maximum
		//
		// And if there is a maximum, clamp swap chain length to it
		if (swap_chain_support.capabilities.maxImageCount > 0 &&
			swapchain_image_count > swap_chain_support.capabilities.maxImageCount)
		{
			swapchain_image_count = swap_chain_support.capabilities.maxImageCount;
			this->logger->SimpleLog(
				Logging::LogLevel::Warning,
				LOGPFX_CURRENT "Swap chain image count limited by maxImageCount capability"
			);
		}

		this->swapchain = new VSwapchain(this->device_manager.get(), extent, swapchain_image_count);

		// this->vk_swap_chain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
		// this->vk_swap_chain_extent = extent;

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

		delete this->multisampled_color_image;
		delete this->vk_depth_buffer;

		// this->CleanupVulkanImage(this->vk_depth_buffer);
		// this->CleanupVulkanImage(this->multisampled_color_image);

		// Create a new swap chain
		this->CreateVulkanSwapChain();
		// this->CreateVulkanSwapChainViews();
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

		delete this->swapchain;
		this->swapchain = nullptr;
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

		pipeline_settings.shader = {"game/shaders/vulkan/default_vert.spv", "game/shaders/vulkan/default_frag.spv"};

		this->graphics_pipeline_default = this->CreateVulkanPipeline(pipeline_settings);
	}

	void VulkanRenderingEngine::CreateVulkanRenderPass()
	{
		VkAttachmentDescription color_attachment{};
		color_attachment.format			= this->swapchain->GetImageFormat();
		color_attachment.samples		= this->device_manager->GetMSAASampleCount();
		color_attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
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
		color_attachment_resolve.format			= this->swapchain->GetImageFormat();
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
		auto& image_view_handles = this->swapchain->GetImageViewHandles();

		this->vk_swap_chain_framebuffers.resize(image_view_handles.size());

		for (size_t i = 0; i < image_view_handles.size(); i++)
		{
			std::array<VkImageView, 3> attachments =
				{this->multisampled_color_image->image_view, this->vk_depth_buffer->image_view, image_view_handles[i]};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass		 = this->vk_render_pass;
			framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebuffer_info.pAttachments	 = attachments.data();
			framebuffer_info.width			 = this->swapchain->GetExtent().width;
			framebuffer_info.height			 = this->swapchain->GetExtent().height;
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

	void VulkanRenderingEngine::CreateVulkanSyncObjects()
	{
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		for (size_t i = 0; i < VulkanRenderingEngine::max_frames_in_flight; i++)
		{
			auto& sync_object_frame = this->sync_objects[i];

			if (vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &(sync_object_frame.image_available)) !=
					VK_SUCCESS ||
				vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &(sync_object_frame.present_ready)) !=
					VK_SUCCESS ||
				vkCreateFence(logical_device, &fence_info, nullptr, &(sync_object_frame.acquire_ready)) != VK_SUCCESS ||
				vkCreateFence(logical_device, &fence_info, nullptr, &(sync_object_frame.in_flight)) != VK_SUCCESS)
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating sync objects");
				throw std::runtime_error("failed to create sync objects!");
			}
		}
	}

	void VulkanRenderingEngine::CreateVulkanDepthResources()
	{
		VkFormat depth_format = this->FindVulkanSupportedDepthFormat();

		this->vk_depth_buffer = new VImage(
			this->device_manager.get(),
			this->vma_allocator,
			{this->swapchain->GetExtent().width, this->swapchain->GetExtent().height},
			this->device_manager->GetMSAASampleCount(),
			depth_format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		this->vk_depth_buffer->AddImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void VulkanRenderingEngine::CreateMultisampledColorResources()
	{
		VkFormat color_format = this->swapchain->GetImageFormat();

		this->multisampled_color_image = new VImage(
			this->device_manager.get(),
			this->vma_allocator,
			{this->swapchain->GetExtent().width, this->swapchain->GetExtent().height},
			this->device_manager->GetMSAASampleCount(),
			color_format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		this->multisampled_color_image->AddImageView(VK_IMAGE_ASPECT_COLOR_BIT);
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
} // namespace Engine::Rendering::Vulkan
