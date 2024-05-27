#include <set>

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

	////////////////////////////////////////////////////////////////////////
	////////////////////////    Init functions    //////////////////////////
	////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::createVulkanSwapChain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swapChainSupport = this->deviceManager->QuerySwapChainSupport();

		// Get the info out of the capabilities
		VkSurfaceFormatKHR surfaceFormat = this->chooseVulkanSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = this->chooseVulkanSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = this->chooseVulkanSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
			this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Using maxImageCount capability, GPU support limited");
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = this->deviceManager->GetSurface();

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Creating Vulkan swap chain with %u images", imageCount);

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices queueIndices = this->deviceManager->GetQueueFamilies();

		uint32_t queueFamilyIndices[] = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};

		if (queueIndices.graphicsFamily != queueIndices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

		if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &(this->vkSwapChain)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan swap chain creation failed");
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(logicalDevice, this->vkSwapChain, &imageCount, nullptr);
		this->vkSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicalDevice, this->vkSwapChain, &imageCount, this->vkSwapChainImages.data());

		this->vkSwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		this->vkSwapChainExtent = extent;

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Vulkan swap chain created");
	}

	void VulkanRenderingEngine::recreateVulkanSwapChain()
	{
		// If window is minimized, wait for it to show up again
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(this->deviceManager->GetLogicalDevice());

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Recreating vulkan swap chain");

		// Clean up old swap chain
		this->cleanupVulkanSwapChain();
		this->cleanupVulkanImage(this->vkDepthBuffer);
		this->cleanupVulkanImage(this->multisampledColorImage);

		// Create a new swap chain
		this->createVulkanSwapChain();
		this->createVulkanSwapChainViews();
		this->createVulkanDepthResources();
		this->createMultisampledColorResources();
		this->createVulkanFramebuffers();
	}

	void VulkanRenderingEngine::cleanupVulkanSwapChain()
	{
		VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

		for (auto framebuffer : this->vkSwapChainFramebuffers)
		{
			vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
		}

		for (auto imageView : this->vkSwapChainImageViews)
		{
			vkDestroyImageView(logicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(logicalDevice, this->vkSwapChain, nullptr);
	}

	void VulkanRenderingEngine::createVulkanSwapChainViews()
	{
		this->vkSwapChainImageViews.resize(this->vkSwapChainImages.size());

		if (const auto &vulkanImageFactory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			for (size_t i = 0; i < this->vkSwapChainImages.size(); i++)
			{
				this->vkSwapChainImageViews[i] = vulkanImageFactory->createImageView(this->vkSwapChainImages[i], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
			}
		}
	}

	VkShaderModule VulkanRenderingEngine::createVulkanShaderModule(const std::vector<char> &code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(this->deviceManager->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating shader module");
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void VulkanRenderingEngine::createVulkanDefaultGraphicsPipeline()
	{
		VulkanPipelineSettings pipeline_settings = this->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		this->graphicsPipelineDefault = this->createVulkanPipeline(pipeline_settings, "game/shaders/vulkan/default_vert.spv", "game/shaders/vulkan/default_frag.spv");
	}

	void VulkanRenderingEngine::createVulkanRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = this->vkSwapChainImageFormat;
		colorAttachment.samples = this->deviceManager->GetMSAASampleCount();
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = this->findVulkanSupportedDepthFormat();
		depthAttachment.samples = this->deviceManager->GetMSAASampleCount();
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = this->vkSwapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(this->deviceManager->GetLogicalDevice(), &renderPassInfo, nullptr, &this->vkRenderPass) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating render pass");
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanRenderingEngine::createVulkanFramebuffers()
	{
		this->vkSwapChainFramebuffers.resize(this->vkSwapChainImageViews.size());

		for (size_t i = 0; i < vkSwapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = {
				this->multisampledColorImage->imageView,
				this->vkDepthBuffer->imageView,
				this->vkSwapChainImageViews[i]};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = this->vkRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = this->vkSwapChainExtent.width;
			framebufferInfo.height = this->vkSwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(this->deviceManager->GetLogicalDevice(), &framebufferInfo, nullptr, &this->vkSwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating framebuffers");
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VulkanRenderingEngine::createVulkanCommandPools()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = this->deviceManager->GetQueueFamilies().graphicsFamily.value();

		if (vkCreateCommandPool(this->deviceManager->GetLogicalDevice(), &poolInfo, nullptr, &(this->vkCommandPool)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating command pools");
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void VulkanRenderingEngine::createVulkanCommandBuffers()
	{
		vkCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->vkCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)vkCommandBuffers.size();

		if (vkAllocateCommandBuffers(this->deviceManager->GetLogicalDevice(), &allocInfo, this->vkCommandBuffers.data()) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating command pools");
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VulkanRenderingEngine::createVulkanSyncObjects()
	{
		this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

			if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &(this->imageAvailableSemaphores[i])) != VK_SUCCESS ||
				vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &(this->renderFinishedSemaphores[i])) != VK_SUCCESS ||
				vkCreateFence(logicalDevice, &fenceInfo, nullptr, &(this->inFlightFences[i])) != VK_SUCCESS)
			{

				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating sync objects");
				throw std::runtime_error("failed to create sync objects!");
			}
		}
	}

	void VulkanRenderingEngine::createVulkanDepthResources()
	{
		VkFormat depthFormat = this->findVulkanSupportedDepthFormat();

		if (const auto &vulkanImageFactory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			this->vkDepthBuffer = vulkanImageFactory->createImage(this->vkSwapChainExtent.width, this->vkSwapChainExtent.height, this->deviceManager->GetMSAASampleCount(), depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			this->vkDepthBuffer->imageView = vulkanImageFactory->createImageView(this->vkDepthBuffer->image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
	}

	void VulkanRenderingEngine::createMultisampledColorResources()
	{
		VkFormat colorFormat = this->vkSwapChainImageFormat;

		if (const auto &vulkanImageFactory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			this->multisampledColorImage = vulkanImageFactory->createImage(this->vkSwapChainExtent.width, this->vkSwapChainExtent.height, this->deviceManager->GetMSAASampleCount(), colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			this->multisampledColorImage->imageView = vulkanImageFactory->createImageView(this->multisampledColorImage->image, this->multisampledColorImage->imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void VulkanRenderingEngine::createVulkanMemoryAllocator()
	{
		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorCreateInfo.physicalDevice = this->deviceManager->GetPhysicalDevice();
		allocatorCreateInfo.device = this->deviceManager->GetLogicalDevice();
		allocatorCreateInfo.instance = this->deviceManager->GetInstance();

		vmaCreateAllocator(&allocatorCreateInfo, &(this->vmaAllocator));

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "VMA created");
	}
}