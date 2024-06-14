#include <algorithm>
#include <fstream>
/*
#define VMA_DEBUG_LOG_FORMAT(format, ...)                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		printf((format), __VA_ARGS__);                                                                                 \
		printf("\n");                                                                                                  \
	} while (false)
 */
#define VMA_IMPLEMENTATION
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

	std::vector<char> VulkanRenderingEngine::ReadShaderFile(std::string path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(file_size));

		file.close();

		return buffer;
	}

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
		app->SignalFramebufferResizeGLFW(width, height);
	}

	////////////////////////////////////////////////////////////////////////
	///////////////////////    Runtime functions    ////////////////////////
	////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::recordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;				   // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &begin_info) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to begin recording command buffer"
			);
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = this->vkRenderPass;
		render_pass_info.framebuffer = this->vkSwapChainFramebuffers[imageIndex];
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = this->vkSwapChainExtent;

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}}; // TODO: configurable
		clear_values[1].depthStencil = {1.0f, 0};

		render_pass_info.clearValueCount = 2;
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(commandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(this->vkSwapChainExtent.width);
		viewport.height = static_cast<float>(this->vkSwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = this->vkSwapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->graphicsPipelineDefault->vk_pipeline_layout,
			0,
			1,
			&this->graphicsPipelineDefault->vk_descriptor_sets[currentFrame],
			0,
			nullptr
		);
		// this->SelectCurrentTopology(commandBuffer, VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST);
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

			VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

			actual_extent.width = std::clamp(
				actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width
			);
			actual_extent.height = std::clamp(
				actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height
			);

			return actual_extent;
		}
	}

	uint32_t VulkanRenderingEngine::findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(this->deviceManager->GetPhysicalDevice(), &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to find required memory type");
		throw std::runtime_error("failed to find required memory type!");
	}

	VkFormat VulkanRenderingEngine::findVulkanSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	)
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
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
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
		VkDevice logical_device = this->deviceManager->GetLogicalDevice();

		vkDeviceWaitIdle(logical_device);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		this->cleanupVulkanSwapChain();

		this->cleanupVulkanImage(this->multisampledColorImage);
		this->cleanupVulkanImage(this->vkDepthBuffer);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(logical_device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(logical_device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(logical_device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(logical_device, this->vkCommandPool, nullptr);

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up default vulkan pipeline");
		this->cleanupVulkanPipeline(this->graphicsPipelineDefault);

		vkDestroyRenderPass(logical_device, this->vkRenderPass, nullptr);

		vmaDestroyAllocator(this->vmaAllocator);

		this->deviceManager->cleanup();

		// Clean up GLFW
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

	void VulkanRenderingEngine::init(unsigned int xsize, unsigned int ysize, std::string title)
	{
		this->window_x = xsize;
		this->window_y = ysize;
		this->window_title = std::move(title);

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
		this->window = glfwCreateWindow(this->window_x, this->window_y, this->window_title.c_str(), nullptr, nullptr);
		glfwSetWindowTitle(this->window, this->window_title.c_str());
		glfwSetWindowSize(this->window, this->window_x, this->window_y);
		glfwSetWindowUserPointer(this->window, this);
		glfwSetFramebufferSizeCallback(this->window, FramebufferResizeCallback);

		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1, LOGPFX_CURRENT "%u vulkan extensions supported", extension_count
		);

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
		VkDevice logical_device = this->deviceManager->GetLogicalDevice();

		vkWaitForFences(logical_device, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t image_index;
		VkResult acquire_result = vkAcquireNextImageKHR(
			logical_device,
			this->vkSwapChain,
			UINT64_MAX,
			this->imageAvailableSemaphores[currentFrame],
			VK_NULL_HANDLE,
			&image_index
		);

		if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || this->framebufferResized ||
			acquire_result == VK_SUBOPTIMAL_KHR)
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

		vkResetFences(logical_device, 1, &this->inFlightFences[currentFrame]);

		vkResetCommandBuffer(this->vkCommandBuffers[currentFrame], 0);
		this->recordVulkanCommandBuffer(this->vkCommandBuffers[currentFrame], image_index);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Wait semaphores
		VkSemaphore wait_semaphores[] = {this->imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &this->vkCommandBuffers[currentFrame];

		// Signal semaphores
		VkSemaphore signal_semaphores[] = {this->renderFinishedSemaphores[currentFrame]};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		if (vkQueueSubmit(
				this->deviceManager->GetGraphicsQueue(), 1, &submit_info, this->inFlightFences[currentFrame]
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		this->currentFrame = (this->currentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = {this->vkSwapChain};
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr; // Optional

		vkQueuePresentKHR(this->deviceManager->GetPresentQueue(), &present_info);
	}

	GLFWwindowData const VulkanRenderingEngine::GetWindow()
	{
		GLFWwindowData data{};
		data.window = this->window;
		data.window_x = this->window_x;
		data.window_y = this->window_y;
		data.window_title = this->window_title;

		return data;
	}

	uint32_t VulkanRenderingEngine::GetMaxFramesInFlight()
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
		this->window_x = width;
		this->window_y = height;
	}

	VkCommandBuffer VulkanRenderingEngine::beginVulkanSingleTimeCommandsCommandBuffer()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = this->vkCommandPool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(this->deviceManager->GetLogicalDevice(), &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void VulkanRenderingEngine::endVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(this->deviceManager->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
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
		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)this->vkSwapChainExtent.width;
		viewport.height = (float)this->vkSwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = this->vkSwapChainExtent;

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;

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
		rasterizer.depthBiasClamp = 0.0f;		   // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f;	   // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = this->deviceManager->GetMSAASampleCount();
		multisampling.minSampleShading = 1.0f;			// Optional
		multisampling.pSampleMask = nullptr;			// Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE;		// Optional

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
												VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending{};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f; // Optional
		color_blending.blendConstants[1] = 0.0f; // Optional
		color_blending.blendConstants[2] = 0.0f; // Optional
		color_blending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depth_stencil{};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.minDepthBounds = 0.0f; // Optional
		depth_stencil.maxDepthBounds = 1.0f; // Optional
		depth_stencil.stencilTestEnable = VK_FALSE;
		depth_stencil.front = {}; // Optional
		depth_stencil.back = {};  // Optional

		VulkanPipelineSettings default_settings{};
		default_settings.inputAssembly = input_assembly;
		default_settings.viewport = viewport;
		default_settings.scissor = scissor;
		default_settings.viewportState = viewport_state;
		default_settings.rasterizer = rasterizer;
		default_settings.multisampling = multisampling;
		default_settings.colorBlendAttachment = color_blend_attachment;
		default_settings.colorBlending = color_blending;
		default_settings.depthStencil = depth_stencil;
		default_settings.descriptorLayoutSettings = {};

		return default_settings;
	}

	VulkanPipeline* VulkanRenderingEngine::createVulkanPipelineFromPrealloc(
		VulkanPipeline* pipeline,
		VulkanPipelineSettings& settings,
		std::string vert_path,
		std::string frag_path
	)
	{
		VkDevice logical_device = this->deviceManager->GetLogicalDevice();

		settings.colorBlending.pAttachments = &settings.colorBlendAttachment;

		auto vert_shader_code = VulkanRenderingEngine::ReadShaderFile(std::move(vert_path));
		auto frag_shader_code = VulkanRenderingEngine::ReadShaderFile(std::move(frag_path));

		VkShaderModule vert_shader_module = this->createVulkanShaderModule(vert_shader_code);
		VkShaderModule frag_shader_module = this->createVulkanShaderModule(frag_shader_code);

		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

		std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		auto binding_description = RenderingVertex::GetBindingDescription();
		auto attribute_descriptions = RenderingVertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_info.pVertexBindingDescriptions = &binding_description;
		vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

		this->createVulkanDescriptorSetLayout(pipeline, settings.descriptorLayoutSettings);

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &pipeline->vk_descriptor_set_layout;
		pipeline_layout_info.pushConstantRangeCount = 0;	// Optional
		pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(logical_device, &pipeline_layout_info, nullptr, &(pipeline->vk_pipeline_layout)) !=
			VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating graphics pipeline layout"
			);
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &settings.inputAssembly;
		pipeline_info.pViewportState = &settings.viewportState;
		pipeline_info.pRasterizationState = &settings.rasterizer;
		pipeline_info.pMultisampleState = &settings.multisampling;
		pipeline_info.pDepthStencilState = nullptr; // Optional
		pipeline_info.pColorBlendState = &settings.colorBlending;
		pipeline_info.pDynamicState = &dynamic_state;
		pipeline_info.layout = pipeline->vk_pipeline_layout;
		pipeline_info.pDepthStencilState = &settings.depthStencil;
		pipeline_info.renderPass = this->vkRenderPass;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipeline_info.basePipelineIndex = -1;			   // Optional

		if (vkCreateGraphicsPipelines(
				logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &(pipeline->pipeline)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed creating triangle graphics pipeline"
			);
			throw std::runtime_error("failed to create triangle graphics pipeline!");
		}

		vkDestroyShaderModule(logical_device, frag_shader_module, nullptr);
		vkDestroyShaderModule(logical_device, vert_shader_module, nullptr);

		this->createVulkanUniformBuffers(pipeline);
		this->createVulkanDescriptorPool(pipeline, settings.descriptorLayoutSettings);
		this->createVulkanDescriptorSets(pipeline, settings.descriptorLayoutSettings);

		return pipeline;
	}

	VulkanPipeline* VulkanRenderingEngine::createVulkanPipeline(
		VulkanPipelineSettings& settings,
		std::string vert_path,
		std::string frag_path
	)
	{
		VulkanPipeline* new_pipeline = new VulkanPipeline();

		this->createVulkanPipelineFromPrealloc(new_pipeline, settings, std::move(vert_path), std::move(frag_path));

		return new_pipeline;
	}

	void VulkanRenderingEngine::cleanupVulkanPipeline(VulkanPipeline* pipeline)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up vulkan pipeline");

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			this->cleanupVulkanBuffer(pipeline->uniform_buffers[i]);
		}

		VkDevice logical_device = this->deviceManager->GetLogicalDevice();

		vkDestroyDescriptorPool(logical_device, pipeline->vk_descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(logical_device, pipeline->vk_descriptor_set_layout, nullptr);

		vkDestroyPipeline(logical_device, pipeline->pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, pipeline->vk_pipeline_layout, nullptr);

		delete pipeline;
	}

	// Buffers

	VulkanBuffer* VulkanRenderingEngine::createVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices)
	{
		VulkanBuffer* staging_buffer{};
		VulkanBuffer* vertex_buffer{};

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

		staging_buffer = this->createVulkanBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0
		);
		vertex_buffer = this->createVulkanBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			0
		);

		vkMapMemory(
			this->deviceManager->GetLogicalDevice(),
			staging_buffer->allocationInfo.deviceMemory,
			staging_buffer->allocationInfo.offset,
			staging_buffer->allocationInfo.size,
			0,
			&staging_buffer->mappedData
		);
		memcpy(staging_buffer->mappedData, vertices.data(), (size_t)buffer_size);
		vkUnmapMemory(this->deviceManager->GetLogicalDevice(), staging_buffer->allocationInfo.deviceMemory);

		this->bufferVulkanTransferCopy(staging_buffer, vertex_buffer, buffer_size);

		this->cleanupVulkanBuffer(staging_buffer);

		return vertex_buffer;
	}

	void VulkanRenderingEngine::createVulkanUniformBuffers(VulkanPipeline* pipeline)
	{
		VkDeviceSize buffer_size = sizeof(glm::mat4);

		pipeline->uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			pipeline->uniform_buffers[i] = this->createVulkanBuffer(
				buffer_size,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				0
			);
		}
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VmaAllocationCreateFlags vmaAllocFlags
	)
	{
		VulkanBuffer* new_buffer = new VulkanBuffer();

		// Create a buffer handle
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
		vma_alloc_info.flags =
			vmaAllocFlags; // VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vma_alloc_info.requiredFlags = properties;

		// if (vkCreateBuffer(this->vkLogicalDevice, &bufferInfo, nullptr, &(new_buffer->buffer)) != VK_SUCCESS)
		if (vmaCreateBuffer(
				this->vmaAllocator,
				&buffer_info,
				&vma_alloc_info,
				&(new_buffer->buffer),
				&(new_buffer->allocation),
				&(new_buffer->allocationInfo)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating buffer");
			throw std::runtime_error("failed to create buffer!");
		}

		vmaSetAllocationName(this->vmaAllocator, new_buffer->allocation, "VulkanBuffer");

		return new_buffer;
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize)
	{
		VulkanBuffer* staging_buffer;

		staging_buffer = this->createVulkanBuffer(
			dataSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0
		);

		vkMapMemory(
			this->deviceManager->GetLogicalDevice(),
			staging_buffer->allocationInfo.deviceMemory,
			staging_buffer->allocationInfo.offset,
			staging_buffer->allocationInfo.size,
			0,
			&staging_buffer->mappedData
		);

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
		// this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up vulkan buffer");

		vkDestroyBuffer(this->deviceManager->GetLogicalDevice(), buffer->buffer, nullptr);
		vmaFreeMemory(this->vmaAllocator, buffer->allocation);

		// Also delete as we use pointers
		delete buffer;
	}

	// Descriptor sets

	void VulkanRenderingEngine::createVulkanDescriptorSetLayout(
		VulkanPipeline* pipeline,
		VulkanDescriptorLayoutSettings settings
	)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings = {};
		std::vector<VkDescriptorBindingFlags> binding_flags = {};
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
			binding_flags.push_back(new_flags);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info{};
		layout_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layout_flags_info.bindingCount = static_cast<uint32_t>(binding_flags.size());
		layout_flags_info.pBindingFlags = binding_flags.data();

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();
		layout_info.pNext = &layout_flags_info;

		if (vkCreateDescriptorSetLayout(
				this->deviceManager->GetLogicalDevice(), &layout_info, nullptr, &(pipeline->vk_descriptor_set_layout)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed to create descriptor set layout"
			);
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanRenderingEngine::createVulkanDescriptorPool(
		VulkanPipeline* pipeline,
		VulkanDescriptorLayoutSettings settings
	)
	{
		std::vector<VkDescriptorPoolSize> pool_sizes{};

		pool_sizes.resize(settings.binding.size());
		for (size_t i = 0; i < settings.binding.size(); i++)
		{
			VkDescriptorPoolSize pool_size{};
			pool_size.type = settings.types[i];
			pool_size.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * settings.descriptorCount[i];

			pool_sizes[i] = pool_size;
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(
				this->deviceManager->GetLogicalDevice(), &pool_info, nullptr, &(pipeline->vk_descriptor_pool)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan failed to create descriptor pool"
			);
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanRenderingEngine::createVulkanDescriptorSets(
		VulkanPipeline* pipeline,
		VulkanDescriptorLayoutSettings settings
	)
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline->vk_descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = pipeline->vk_descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		alloc_info.pSetLayouts = layouts.data();

		pipeline->vk_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult create_result{};
		if ((create_result = vkAllocateDescriptorSets(
				 this->deviceManager->GetLogicalDevice(), &alloc_info, pipeline->vk_descriptor_sets.data()
			 )) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "Vulkan failed to create descriptor sets, VkResult: %i",
				create_result
			);
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	// Image functions

	void VulkanRenderingEngine::copyVulcanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer command_buffer = this->beginVulkanSingleTimeCommandsCommandBuffer();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		this->endVulkanSingleTimeCommandsCommandBuffer(command_buffer);
	}

	void VulkanRenderingEngine::appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage)
	{
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = teximage->useFilter;
		sampler_info.minFilter = teximage->useFilter; // VK_FILTER_LINEAR;

		sampler_info.addressModeU = teximage->useAddressMode; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = teximage->useAddressMode;
		sampler_info.addressModeW = teximage->useAddressMode;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(this->deviceManager->GetPhysicalDevice(), &properties);

		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		sampler_info.unnormalizedCoordinates = VK_FALSE;

		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;

		if (vkCreateSampler(
				this->deviceManager->GetLogicalDevice(), &sampler_info, nullptr, &(teximage->textureSampler)
			) != VK_SUCCESS)
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
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up Vulkan texture image");

		vkDestroySampler(this->deviceManager->GetLogicalDevice(), image->textureSampler, nullptr);

		this->cleanupVulkanImage(image->image);

		// Also delete as we use pointers
		delete image;
	}

} // namespace Engine::Rendering
