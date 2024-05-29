#include <algorithm>
#include <set>
#include <fstream>

#define VMA_IMPLEMENTATION
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

	std::vector<char> VulkanRenderingEngine::readShaderFile(std::string path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
		app->SignalFramebufferResizeGLFW(width, height);
	}

////////////////////////////////////////////////////////////////////////
///////////////////////    Runtime functions    ////////////////////////
////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::recordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to begin recording command buffer");
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->vkRenderPass;
		renderPassInfo.framebuffer = this->vkSwapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = this->vkSwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} }; // todo configurable
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(this->vkSwapChainExtent.width);
		viewport.height = static_cast<float>(this->vkSwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = this->vkSwapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipelineDefault->vkPipelineLayout, 0, 1, &this->graphicsPipelineDefault->vkDescriptorSets[currentFrame], 0, nullptr);
		//this->SelectCurrentTopology(commandBuffer, VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipelineDefault->pipeline);
		if (this->external_callback)
		{
			this->external_callback(commandBuffer, currentFrame, this->owner_engine);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to record command buffer");
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	VkExtent2D VulkanRenderingEngine::chooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	
	uint32_t VulkanRenderingEngine::findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(this->deviceManager->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to find required memory type");
		throw std::runtime_error("failed to find required memory type!");
	}

	VkFormat VulkanRenderingEngine::findVulkanSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(this->deviceManager->GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to find supported vkFormat");
		throw std::runtime_error("failed to find supported vkFormat!");
	}

	VkFormat VulkanRenderingEngine::findVulkanSupportedDepthFormat()
	{
		return this->findVulkanSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

////////////////////////////////////////////////////////////////////////
////////////////////////    Init functions    //////////////////////////
////////////////////////////////////////////////////////////////////////

	// Section moved to 'VulkanRenderingEngine_Init.cpp'
	//

////////////////////////////////////////////////////////////////////////
///////////////////////    Public Interface    /////////////////////////
////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::cleanup()
	{
		VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

		vkDeviceWaitIdle(logicalDevice);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		this->cleanupVulkanSwapChain();

		this->cleanupVulkanImage(this->multisampledColorImage);
		this->cleanupVulkanImage(this->vkDepthBuffer);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(logicalDevice, this->vkCommandPool, nullptr);

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up default vulkan pipeline");
		this->cleanupVulkanPipeline(this->graphicsPipelineDefault);

		vkDestroyRenderPass(logicalDevice, this->vkRenderPass, nullptr);
		
		vmaDestroyAllocator(this->vmaAllocator);

		this->deviceManager->cleanup();

		// Clean up GLFW
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

	void VulkanRenderingEngine::init(unsigned int xsize, unsigned int ysize, std::string title)
	{
		this->windowX = xsize;
		this->windowY = ysize;
		this->windowTitle = std::move(title);

		// Initialize GLFW
		if (glfwInit() == GLFW_FALSE)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "glfwInit returned GLFW_FALSE!");
			exit(1);
		}
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "GLFW initialized");

		// Create a GLFW window
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		this->window = glfwCreateWindow(this->windowX, this->windowY, this->windowTitle.c_str(), NULL, NULL);
		glfwSetWindowTitle(this->window, this->windowTitle.c_str());
		glfwSetWindowSize(this->window, this->windowX, this->windowY);
		glfwSetWindowUserPointer(this->window, this);
		glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "%u vulkan extensions supported", extensionCount);

		// Set up our vulkan rendering stack
		this->deviceManager = std::make_unique<VulkanDeviceManager>();
		this->deviceManager->UpdateHeldLogger(this->logger);
		this->deviceManager->UpdateOwnerEngine(this->owner_engine);

		this->deviceManager->init(this->window);

		this->createVulkanMemoryAllocator();
	}
	
	void VulkanRenderingEngine::prepRun()
	{
		this->createVulkanSwapChain();
		this->createVulkanSwapChainViews();
		this->createVulkanRenderPass();
		this->createVulkanDefaultGraphicsPipeline();
		this->createVulkanCommandPools();
		this->createVulkanCommandBuffers();
		this->createMultisampledColorResources();
		this->createVulkanDepthResources();
		this->createVulkanFramebuffers();
		this->createVulkanSyncObjects();
	}

	void VulkanRenderingEngine::drawFrame()
	{
		VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

		vkWaitForFences(logicalDevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult acquire_result = vkAcquireNextImageKHR(logicalDevice, this->vkSwapChain, UINT64_MAX, this->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || this->framebufferResized || acquire_result == VK_SUBOPTIMAL_KHR)
		{
			this->currentFrame = (this->currentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;
			this->framebufferResized = false;

			this->recreateVulkanSwapChain();
			
			return;
		}
		else if (acquire_result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(logicalDevice, 1, &this->inFlightFences[currentFrame]);

		vkResetCommandBuffer(this->vkCommandBuffers[currentFrame], 0);
		this->recordVulkanCommandBuffer(this->vkCommandBuffers[currentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Wait semaphores
		VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &this->vkCommandBuffers[currentFrame];

		// Signal semaphores
		VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(this->deviceManager->GetGraphicsQueue(), 1, &submitInfo, this->inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		this->currentFrame = (this->currentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { this->vkSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(this->deviceManager->GetPresentQueue(), &presentInfo);
	}

	GLFWwindowData const VulkanRenderingEngine::GetWindow()
	{
		GLFWwindowData data{};
		data.window = this->window;
		data.windowX = this->windowX;
		data.windowY = this->windowY;
		data.windowTitle = this->windowTitle;

		return data;
	}

	const uint32_t VulkanRenderingEngine::GetMaxFramesInFlight()
	{
		return this->MAX_FRAMES_IN_FLIGHT;
	}

	VmaAllocator VulkanRenderingEngine::GetVMAAllocator()
	{
		return this->vmaAllocator;
	}

	void VulkanRenderingEngine::SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Engine*)> callback)
	{
		this->external_callback = std::move(callback);
	}

	void VulkanRenderingEngine::SignalFramebufferResizeGLFW(int width, int height)
	{
		this->framebufferResized = true;
		this->windowX = width;
		this->windowY = height;
	}

	VkCommandBuffer VulkanRenderingEngine::beginVulkanSingleTimeCommandsCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->vkCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(this->deviceManager->GetLogicalDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanRenderingEngine::endVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(this->deviceManager->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(this->deviceManager->GetGraphicsQueue());

		vkFreeCommandBuffers(this->deviceManager->GetLogicalDevice(), this->vkCommandPool, 1, &commandBuffer);
	}

	VkDevice VulkanRenderingEngine::GetLogicalDevice()
	{
		return this->deviceManager->GetLogicalDevice();
	}

	// Pipelines

	VulkanPipelineSettings VulkanRenderingEngine::getVulkanDefaultPipelineSettings()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)this->vkSwapChainExtent.width;
		viewport.height = (float)this->vkSwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = this->vkSwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = this->deviceManager->GetMSAASampleCount();
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VulkanPipelineSettings default_settings{};
		default_settings.inputAssembly = inputAssembly;
		default_settings.viewport = viewport;
		default_settings.scissor = scissor;
		default_settings.viewportState = viewportState;
		default_settings.rasterizer = rasterizer;
		default_settings.multisampling = multisampling;
		default_settings.colorBlendAttachment = colorBlendAttachment;
		default_settings.colorBlending = colorBlending;
		default_settings.depthStencil = depthStencil;
		default_settings.descriptorLayoutSettings = {};

		return default_settings;
	}

	VulkanPipeline* VulkanRenderingEngine::createVulkanPipelineFromPrealloc(VulkanPipeline* pipeline, VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path)
	{
		VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

		settings.colorBlending.pAttachments = &settings.colorBlendAttachment;

		auto vertShaderCode = VulkanRenderingEngine::readShaderFile(std::move(vert_path));
		auto fragShaderCode = VulkanRenderingEngine::readShaderFile(std::move(frag_path));

		VkShaderModule vertShaderModule = this->createVulkanShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = this->createVulkanShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		auto bindingDescription = RenderingVertex::getBindingDescription();
		auto attributeDescriptions = RenderingVertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		this->createVulkanDescriptorSetLayout(pipeline, settings.descriptorLayoutSettings);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &pipeline->vkDescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &(pipeline->vkPipelineLayout)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating graphics pipeline layout");
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &settings.inputAssembly;
		pipelineInfo.pViewportState = &settings.viewportState;
		pipelineInfo.pRasterizationState = &settings.rasterizer;
		pipelineInfo.pMultisampleState = &settings.multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &settings.colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline->vkPipelineLayout;
		pipelineInfo.pDepthStencilState = &settings.depthStencil;
		pipelineInfo.renderPass = this->vkRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &(pipeline->pipeline)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed creating triangle graphics pipeline");
			throw std::runtime_error("failed to create triangle graphics pipeline!");
		}

		vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);

		this->createVulkanUniformBuffers(pipeline);
		this->createVulkanDescriptorPool(pipeline, settings.descriptorLayoutSettings);
		this->createVulkanDescriptorSets(pipeline, settings.descriptorLayoutSettings);

		return pipeline;
	}

	VulkanPipeline* VulkanRenderingEngine::createVulkanPipeline(VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path)
	{
		VulkanPipeline* new_pipeline = new VulkanPipeline();

		this->createVulkanPipelineFromPrealloc(new_pipeline, settings, std::move(vert_path), std::move(frag_path));

		return new_pipeline;
	}

	void VulkanRenderingEngine::cleanupVulkanPipeline(VulkanPipeline* pipeline)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up vulkan pipeline");

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			this->cleanupVulkanBuffer(pipeline->uniformBuffers[i]);
		}

		VkDevice logicalDevice = this->deviceManager->GetLogicalDevice();

		vkDestroyDescriptorPool(logicalDevice, pipeline->vkDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(logicalDevice, pipeline->vkDescriptorSetLayout, nullptr);

		vkDestroyPipeline(logicalDevice, pipeline->pipeline, nullptr);
		vkDestroyPipelineLayout(logicalDevice, pipeline->vkPipelineLayout, nullptr);

		delete pipeline;
	}


	// Buffers

	VulkanBuffer* VulkanRenderingEngine::createVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices)
	{
		VulkanBuffer* staging_buffer{};
		VulkanBuffer* vertex_buffer{};

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		staging_buffer = this->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		vertex_buffer = this->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		
		vkMapMemory(this->deviceManager->GetLogicalDevice(), staging_buffer->allocationInfo.deviceMemory, staging_buffer->allocationInfo.offset, staging_buffer->allocationInfo.size, 0, &staging_buffer->mappedData);
		memcpy(staging_buffer->mappedData, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(this->deviceManager->GetLogicalDevice(), staging_buffer->allocationInfo.deviceMemory);

		this->bufferVulkanTransferCopy(staging_buffer, vertex_buffer, bufferSize);

		this->cleanupVulkanBuffer(staging_buffer);

		return vertex_buffer;
	}

	void VulkanRenderingEngine::createVulkanUniformBuffers(VulkanPipeline* pipeline)
	{
		VkDeviceSize bufferSize = sizeof(glm::mat4);

		pipeline->uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			pipeline->uniformBuffers[i] = this->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		}
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlags vmaAllocFlags)
	{
		VulkanBuffer* new_buffer = new VulkanBuffer();

		// Create a buffer handle
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaAllocInfo.flags = vmaAllocFlags; //VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vmaAllocInfo.requiredFlags = properties;

		//if (vkCreateBuffer(this->vkLogicalDevice, &bufferInfo, nullptr, &(new_buffer->buffer)) != VK_SUCCESS)
		if (vmaCreateBuffer(this->vmaAllocator, &bufferInfo, &vmaAllocInfo, &(new_buffer->buffer), &(new_buffer->allocation), &(new_buffer->allocationInfo)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating buffer");
			throw std::runtime_error("failed to create buffer!");
		}

		return new_buffer;
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize)
	{
		VulkanBuffer* staging_buffer;

		staging_buffer = this->createVulkanBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);

		vkMapMemory(this->deviceManager->GetLogicalDevice(), staging_buffer->allocationInfo.deviceMemory, staging_buffer->allocationInfo.offset, staging_buffer->allocationInfo.size, 0, &staging_buffer->mappedData);
		
		memcpy(staging_buffer->mappedData, data, static_cast<size_t>(dataSize));

		vkUnmapMemory(this->deviceManager->GetLogicalDevice(), staging_buffer->allocationInfo.deviceMemory);
		
		return staging_buffer;
	}

	void VulkanRenderingEngine::bufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = this->beginVulkanSingleTimeCommandsCommandBuffer();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src->buffer, dest->buffer, 1, &copyRegion);

		this->endVulkanSingleTimeCommandsCommandBuffer(commandBuffer);
	}

	void VulkanRenderingEngine::cleanupVulkanBuffer(VulkanBuffer* buffer)
	{
		//this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up vulkan buffer");

		vkDestroyBuffer(this->deviceManager->GetLogicalDevice(), buffer->buffer, nullptr);
		vmaFreeMemory(this->vmaAllocator, buffer->allocation);

		// Also delete as we use pointers
		delete buffer;
	}


	// Descriptor sets

	void VulkanRenderingEngine::createVulkanDescriptorSetLayout(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {};
		std::vector<VkDescriptorBindingFlags> bindingFlags = {};
		for (size_t i = 0; i < settings.binding.size(); i++)
		{
			VkDescriptorSetLayoutBinding new_binding{};
			new_binding.binding = settings.binding[i];
			new_binding.descriptorCount = settings.descriptorCount[i];
			new_binding.descriptorType = settings.types[i];
			new_binding.stageFlags = settings.stageFlags[i];
			new_binding.pImmutableSamplers = nullptr;

			bindings.push_back(new_binding);

			VkDescriptorBindingFlags new_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			bindingFlags.push_back(new_flags);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo layoutFlagsInfo{};
		layoutFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layoutFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
		layoutFlagsInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.pNext = &layoutFlagsInfo;

		if (vkCreateDescriptorSetLayout(this->deviceManager->GetLogicalDevice(), &layoutInfo, nullptr, &(pipeline->vkDescriptorSetLayout)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed to create descriptor set layout");
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanRenderingEngine::createVulkanDescriptorPool(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings)
	{
		std::vector<VkDescriptorPoolSize> poolSizes{};

		poolSizes.resize(settings.binding.size());
		for (int i = 0; i < settings.binding.size(); i++)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = settings.types[i];
			poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * settings.descriptorCount[i];

			poolSizes[i] = poolSize;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(this->deviceManager->GetLogicalDevice(), &poolInfo, nullptr, &(pipeline->vkDescriptorPool)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed to create descriptor pool");
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanRenderingEngine::createVulkanDescriptorSets(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings)
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline->vkDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pipeline->vkDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		pipeline->vkDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult createResult{};
		if ((createResult = vkAllocateDescriptorSets(this->deviceManager->GetLogicalDevice(), &allocInfo, pipeline->vkDescriptorSets.data())) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed to create descriptor sets, VkResult: %i", createResult);
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}


	// Image functions

	void VulkanRenderingEngine::copyVulcanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = this->beginVulkanSingleTimeCommandsCommandBuffer();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		this->endVulkanSingleTimeCommandsCommandBuffer(commandBuffer);
	}

	void VulkanRenderingEngine::appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = teximage->useFilter;
		samplerInfo.minFilter = teximage->useFilter;//VK_FILTER_LINEAR;

		samplerInfo.addressModeU = teximage->useAddressMode;//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = teximage->useAddressMode;
		samplerInfo.addressModeW = teximage->useAddressMode;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(this->deviceManager->GetPhysicalDevice(), &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(this->deviceManager->GetLogicalDevice(), &samplerInfo, nullptr, &(teximage->textureSampler)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Failed to create texture sampler");
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
	
	void VulkanRenderingEngine::cleanupVulkanImage(VulkanImage* image)
	{
		vkDestroyImageView(this->deviceManager->GetLogicalDevice(), image->imageView, nullptr);
		vkDestroyImage(this->deviceManager->GetLogicalDevice(), image->image, nullptr);
		vmaFreeMemory(this->vmaAllocator, image->allocation);

		// Also delete as we use pointers
		delete image;
	}

	void VulkanRenderingEngine::cleanupVulkanTextureImage(VulkanTextureImage* image)
	{
		//this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up Vulkan texture image");

		vkDestroySampler(this->deviceManager->GetLogicalDevice(), image->textureSampler, nullptr);
		
		this->cleanupVulkanImage(image->image);

		// Also delete as we use pointers
		delete image;
	}

}