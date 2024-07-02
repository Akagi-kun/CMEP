#include "vulkan/vulkan_core.h"

#include <algorithm>
#include <cstdint>

/*
#define VMA_DEBUG_LOG_FORMAT(format, ...)                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		printf((format), __VA_ARGS__);                                                                                 \
		printf("\n");                                                                                                  \
	} while (false)
 */
#define VMA_IMPLEMENTATION
#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/VulkanDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto* app = reinterpret_cast<VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
		app->SignalFramebufferResizeGLFW({static_cast<uint_fast16_t>(width), static_cast<uint_fast16_t>(height)});
	}

	////////////////////////////////////////////////////////////////////////
	///////////////////////    Runtime functions    ////////////////////////
	////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::RecordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags			= 0;	   // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &begin_info) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "Failed to begin recording command buffer"
			);
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType			   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass		   = this->vk_render_pass;
		render_pass_info.framebuffer	   = this->vk_swap_chain_framebuffers[imageIndex];
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = this->vk_swap_chain_extent;

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color		 = {{0.0f, 0.0f, 0.0f, 1.0f}}; // TODO: configurable
		clear_values[1].depthStencil = {1.0f, 0};

		render_pass_info.clearValueCount = 2;
		render_pass_info.pClearValues	 = clear_values.data();

		vkCmdBeginRenderPass(commandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x		  = 0.0f;
		viewport.y		  = 0.0f;
		viewport.width	  = static_cast<float>(this->vk_swap_chain_extent.width);
		viewport.height	  = static_cast<float>(this->vk_swap_chain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = this->vk_swap_chain_extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->graphics_pipeline_default->vk_pipeline_layout,
			0,
			1,
			&this->graphics_pipeline_default->vk_descriptor_sets[current_frame],
			0,
			nullptr
		);

		// Perform actual render
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline_default->pipeline);
		if (this->external_callback)
		{
			this->external_callback(commandBuffer, current_frame, this->owner_engine);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to record command buffer");
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	VkExtent2D VulkanRenderingEngine::ChooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int width;
		int height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		actual_extent.width = std::clamp(
			actual_extent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actual_extent.height = std::clamp(
			actual_extent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actual_extent;
	}

	uint32_t VulkanRenderingEngine::FindVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(this->device_manager->GetPhysicalDevice(), &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) != 0 &&
				(mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to find required memory type");
		throw std::runtime_error("failed to find required memory type!");
	}

	VkFormat VulkanRenderingEngine::FindVulkanSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(this->device_manager->GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to find supported vkFormat");
		throw std::runtime_error("failed to find supported vkFormat!");
	}

	VkFormat VulkanRenderingEngine::FindVulkanSupportedDepthFormat()
	{
		return this->FindVulkanSupportedFormat(
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

	void VulkanRenderingEngine::Cleanup()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		vkDeviceWaitIdle(logical_device);

		this->CleanupVulkanSwapChain();

		this->CleanupVulkanImage(this->multisampled_color_image);
		this->CleanupVulkanImage(this->vk_depth_buffer);

		for (size_t i = 0; i < VulkanRenderingEngine::max_frames_in_flight; i++)
		{
			vkDestroySemaphore(logical_device, this->present_ready_semaphores[i], nullptr);
			vkDestroySemaphore(logical_device, this->image_available_semaphores[i], nullptr);
			vkDestroyFence(logical_device, this->in_flight_fences[i], nullptr);

			vkWaitForFences(logical_device, 1, &this->acquire_ready_fences[i], VK_TRUE, UINT64_MAX);
			vkDestroyFence(logical_device, this->acquire_ready_fences[i], nullptr);
		}

		vkDestroyCommandPool(logical_device, this->vk_command_pool, nullptr);

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up default vulkan pipeline");
		this->CleanupVulkanPipeline(this->graphics_pipeline_default);

		vkDestroyRenderPass(logical_device, this->vk_render_pass, nullptr);

		// VMA Cleanup
		vmaDestroyAllocator(this->vma_allocator);

		// Destroy device after VMA
		this->device_manager->Cleanup();

		// Clean up GLFW
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

	void VulkanRenderingEngine::Init(unsigned int xsize, unsigned int ysize, std::string title)
	{
		this->window_size.x = xsize;
		this->window_size.y = ysize;
		this->window_title	= std::move(title);

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

		// Quick hack for i3wm
		// (this one is likely unnecessary and can be left commented out)
		// glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
		//
		// TODO: Fix i3wm
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		this->window = glfwCreateWindow(
			static_cast<int>(this->window_size.x),
			static_cast<int>(this->window_size.y),
			this->window_title.c_str(),
			nullptr,
			nullptr
		);
		glfwSetWindowTitle(this->window, this->window_title.c_str());
		glfwSetWindowSize(this->window, static_cast<int>(this->window_size.x), static_cast<int>(this->window_size.y));
		glfwSetWindowUserPointer(this->window, this);
		glfwSetFramebufferSizeCallback(this->window, FramebufferResizeCallback);

		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "%u vulkan extensions supported", extension_count);

		// Set up our vulkan rendering stack
		this->device_manager = std::make_unique<VulkanDeviceManager>();
		this->device_manager->UpdateHeldLogger(this->logger);
		this->device_manager->UpdateOwnerEngine(this->owner_engine);

		this->device_manager->Init(this->window);

		this->CreateVulkanMemoryAllocator();
	}

	void VulkanRenderingEngine::PrepRun()
	{
		this->CreateVulkanSwapChain();
		this->CreateVulkanSwapChainViews();
		this->CreateVulkanRenderPass();
		this->CreateVulkanDefaultGraphicsPipeline();
		this->CreateVulkanCommandPools();
		this->CreateVulkanCommandBuffers();
		this->CreateMultisampledColorResources();
		this->CreateVulkanDepthResources();
		this->CreateVulkanFramebuffers();
		this->CreateVulkanSyncObjects();
	}

	void VulkanRenderingEngine::DrawFrame()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		// Wait for fence
		vkWaitForFences(logical_device, 1, &this->in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
		vkWaitForFences(logical_device, 1, &this->acquire_ready_fences[current_frame], VK_TRUE, UINT64_MAX);

		// Reset fence after wait is over
		// (fence has to be reset before being used again)
		vkResetFences(logical_device, 1, &this->in_flight_fences[current_frame]);
		vkResetFences(logical_device, 1, &this->acquire_ready_fences[current_frame]);

		// Index of framebuffer in this->vk_swap_chain_framebuffers
		uint32_t image_index;
		// Acquire render target
		// the render target is an image in the swap chain
		VkResult acquire_result = vkAcquireNextImageKHR(
			logical_device,
			this->vk_swap_chain,
			UINT64_MAX,
			this->image_available_semaphores[current_frame],
			this->acquire_ready_fences[current_frame],
			&image_index
		);

		// If it's necessary to recreate a swap chain
		if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || this->framebuffer_resized ||
			acquire_result == VK_SUBOPTIMAL_KHR)
		{
			// Increment current_frame (clamp to max_frames_in_flight)
			// this->current_frame		  = (this->current_frame + 1) % this->max_frames_in_flight;
			this->framebuffer_resized = false;

			this->logger->SimpleLog(
				Logging::LogLevel::Warning,
				"acquire_result was VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR, or framebuffer was resized!"
			);

			this->RecreateVulkanSwapChain();

			return;
		}

		if (acquire_result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// Reset command buffer to initial state
		vkResetCommandBuffer(this->vk_command_buffers[current_frame], 0);

		// Records render into command buffer
		this->RecordVulkanCommandBuffer(this->vk_command_buffers[current_frame], image_index);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Wait semaphores
		VkSemaphore wait_semaphores[]	   = {this->image_available_semaphores[current_frame]};
		VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount	   = 1;
		submit_info.pWaitSemaphores		   = wait_semaphores;
		submit_info.pWaitDstStageMask	   = wait_stages;
		submit_info.commandBufferCount	   = 1;
		submit_info.pCommandBuffers		   = &this->vk_command_buffers[current_frame];

		// Signal semaphores to be signaled once
		// all submit_info.pCommandBuffers finish executing
		// present_ready_semaphores are used in the next step
		VkSemaphore signal_semaphores[]	 = {this->present_ready_semaphores[current_frame]};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores	 = signal_semaphores;

		// Submit to queue
		// in_flight_fences[current_frame] will be signaled once
		// all submit_info.pCommandBuffers finish executing
		if (vkQueueSubmit(
				this->device_manager->GetGraphicsQueue(),
				1,
				&submit_info,
				this->in_flight_fences[current_frame]
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// Increment current frame
		this->current_frame = 0;
		// this->current_frame = (this->current_frame + 1) % this->max_frames_in_flight;

		VkPresentInfoKHR present_info{};
		present_info.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		// Wait for present_ready_semaphores to be signaled
		// (when signaled = image is ready to be presented)
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores	= signal_semaphores;

		VkSwapchainKHR swap_chains[] = {this->vk_swap_chain};
		present_info.swapchainCount	 = 1;
		present_info.pSwapchains	 = swap_chains;
		present_info.pImageIndices	 = &image_index;
		present_info.pResults		 = nullptr; // Optional

		// Present current image to the screen
		vkQueuePresentKHR(this->device_manager->GetPresentQueue(), &present_info);
	}

	GLFWwindowData VulkanRenderingEngine::GetWindow() const
	{
		GLFWwindowData data{};
		data.window		  = this->window;
		data.window_x	  = this->window_size.x;
		data.window_y	  = this->window_size.y;
		data.window_title = this->window_title;

		return data;
	}

	std::weak_ptr<VulkanDeviceManager> VulkanRenderingEngine::GetDeviceManager()
	{
		return this->device_manager;
	}

	uint32_t VulkanRenderingEngine::GetMaxFramesInFlight()
	{
		return VulkanRenderingEngine::max_frames_in_flight;
	}

	VmaAllocator VulkanRenderingEngine::GetVMAAllocator()
	{
		return this->vma_allocator;
	}

	void VulkanRenderingEngine::SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Engine*)> callback)
	{
		this->external_callback = std::move(callback);
	}

	void VulkanRenderingEngine::SyncDeviceWaitIdle()
	{
		vkDeviceWaitIdle(this->device_manager->GetLogicalDevice());
	}

	void VulkanRenderingEngine::SignalFramebufferResizeGLFW(ScreenSize with_size)
	{
		this->framebuffer_resized = true;
		this->window_size		  = with_size;
	}

	VkCommandBuffer VulkanRenderingEngine::BeginSingleTimeCommandBuffer()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool		  = this->vk_command_pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(this->device_manager->GetLogicalDevice(), &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void VulkanRenderingEngine::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submit_info{};
		submit_info.sType			   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers	   = &commandBuffer;

		vkQueueSubmit(this->device_manager->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(this->device_manager->GetGraphicsQueue());

		vkFreeCommandBuffers(this->device_manager->GetLogicalDevice(), this->vk_command_pool, 1, &commandBuffer);
	}

	// Pipelines

	VulkanPipelineSettings VulkanRenderingEngine::GetVulkanDefaultPipelineSettings()
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType				  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology				  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x		  = 0.0f;
		viewport.y		  = 0.0f;
		viewport.width	  = static_cast<float>(this->vk_swap_chain_extent.width);
		viewport.height	  = static_cast<float>(this->vk_swap_chain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = this->vk_swap_chain_extent;

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType		 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports	 = &viewport;
		viewport_state.scissorCount	 = 1;
		viewport_state.pScissors	 = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType				   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable		   = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode			   = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth			   = 1.0f;
		rasterizer.cullMode				   = VK_CULL_MODE_FRONT_BIT;
		rasterizer.frontFace			   = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable		   = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp		   = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor	   = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable	= VK_FALSE;
		multisampling.rasterizationSamples	= this->device_manager->GetMSAASampleCount();
		multisampling.minSampleShading		= 1.0f;		// Optional
		multisampling.pSampleMask			= nullptr;	// Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable		= VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
												VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable		   = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp		   = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp		   = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending{};
		color_blending.sType			 = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable	 = VK_FALSE;
		color_blending.logicOp			 = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount	 = 1;
		color_blending.pAttachments		 = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f; // Optional
		color_blending.blendConstants[1] = 0.0f; // Optional
		color_blending.blendConstants[2] = 0.0f; // Optional
		color_blending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depth_stencil{};
		depth_stencil.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable		= VK_TRUE;
		depth_stencil.depthWriteEnable		= VK_TRUE;
		depth_stencil.depthCompareOp		= VK_COMPARE_OP_LESS;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.minDepthBounds		= 0.0f; // Optional
		depth_stencil.maxDepthBounds		= 1.0f; // Optional
		depth_stencil.stencilTestEnable		= VK_FALSE;
		depth_stencil.front					= {}; // Optional
		depth_stencil.back					= {}; // Optional

		VulkanPipelineSettings default_settings{};
		default_settings.input_assembly				= input_assembly;
		default_settings.viewport					= viewport;
		default_settings.scissor					= scissor;
		default_settings.viewport_state				= viewport_state;
		default_settings.rasterizer					= rasterizer;
		default_settings.multisampling				= multisampling;
		default_settings.color_blend_attachment		= color_blend_attachment;
		default_settings.color_blending				= color_blending;
		default_settings.depth_stencil				= depth_stencil;
		default_settings.descriptor_layout_settings = {};

		return default_settings;
	}

	VulkanPipeline* VulkanRenderingEngine::CreateVulkanPipelineFromPrealloc(
		VulkanPipeline* pipeline,
		VulkanPipelineSettings& settings
	)
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		settings.color_blending.pAttachments = &settings.color_blend_attachment;

		// Vertex stage
		assert(settings.shader.vertex_stage != nullptr && "A valid shader for this stage is required!");
		auto vert_shader_code			  = VulkanUtils::ReadShaderFile(settings.shader.vertex_stage);
		VkShaderModule vert_shader_module = this->CreateVulkanShaderModule(vert_shader_code);

		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName  = "main";

		// Fragment stage
		assert(settings.shader.fragment_stage != nullptr && "A valid shader for this stage is required!");
		auto frag_shader_code			  = VulkanUtils::ReadShaderFile(settings.shader.fragment_stage);
		VkShaderModule frag_shader_module = this->CreateVulkanShaderModule(frag_shader_code);

		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName  = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
		std::vector<VkDynamicState> dynamic_states		= {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates	= dynamic_states.data();

		auto binding_description	= RenderingVertex::GetBindingDescription();
		auto attribute_descriptions = RenderingVertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType							  = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount	  = 1;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_info.pVertexBindingDescriptions	  = &binding_description;
		vertex_input_info.pVertexAttributeDescriptions	  = attribute_descriptions.data();

		this->CreateVulkanDescriptorSetLayout(pipeline, settings.descriptor_layout_settings);

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount			= 1;
		pipeline_layout_info.pSetLayouts			= &pipeline->vk_descriptor_set_layout;
		pipeline_layout_info.pushConstantRangeCount = 0;	   // Optional
		pipeline_layout_info.pPushConstantRanges	= nullptr; // Optional

		if (vkCreatePipelineLayout(logical_device, &pipeline_layout_info, nullptr, &(pipeline->vk_pipeline_layout)) !=
			VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error,
				LOGPFX_CURRENT "Vulkan failed creating graphics pipeline layout"
			);
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType				  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount		  = 2;
		pipeline_info.pStages			  = shader_stages;
		pipeline_info.pVertexInputState	  = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &settings.input_assembly;
		pipeline_info.pViewportState	  = &settings.viewport_state;
		pipeline_info.pRasterizationState = &settings.rasterizer;
		pipeline_info.pMultisampleState	  = &settings.multisampling;
		pipeline_info.pDepthStencilState  = nullptr; // Optional
		pipeline_info.pColorBlendState	  = &settings.color_blending;
		pipeline_info.pDynamicState		  = &dynamic_state;
		pipeline_info.layout			  = pipeline->vk_pipeline_layout;
		pipeline_info.pDepthStencilState  = &settings.depth_stencil;
		pipeline_info.renderPass		  = this->vk_render_pass;
		pipeline_info.subpass			  = 0;
		pipeline_info.basePipelineHandle  = VK_NULL_HANDLE; // Optional
		pipeline_info.basePipelineIndex	  = -1;				// Optional

		if (vkCreateGraphicsPipelines(
				logical_device,
				VK_NULL_HANDLE,
				1,
				&pipeline_info,
				nullptr,
				&(pipeline->pipeline)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "Vulkan failed creating triangle graphics pipeline"
			);
			throw std::runtime_error("failed to create triangle graphics pipeline!");
		}

		vkDestroyShaderModule(logical_device, frag_shader_module, nullptr);
		vkDestroyShaderModule(logical_device, vert_shader_module, nullptr);

		this->CreateVulkanUniformBuffers(pipeline);
		this->CreateVulkanDescriptorPool(pipeline, settings.descriptor_layout_settings);
		this->CreateVulkanDescriptorSets(pipeline);

		return pipeline;
	}

	VulkanPipeline* VulkanRenderingEngine::CreateVulkanPipeline(VulkanPipelineSettings& settings)
	{
		auto* new_pipeline = new VulkanPipeline();

		this->CreateVulkanPipelineFromPrealloc(new_pipeline, settings);

		return new_pipeline;
	}

	void VulkanRenderingEngine::CleanupVulkanPipeline(VulkanPipeline* pipeline)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up vulkan pipeline");

		for (size_t i = 0; i < VulkanRenderingEngine::max_frames_in_flight; i++)
		{
			this->CleanupVulkanBuffer(pipeline->uniform_buffers[i]);
		}

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		vkDestroyDescriptorPool(logical_device, pipeline->vk_descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(logical_device, pipeline->vk_descriptor_set_layout, nullptr);

		vkDestroyPipeline(logical_device, pipeline->pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, pipeline->vk_pipeline_layout, nullptr);

		delete pipeline;
	}

	// Buffers

	VulkanBuffer* VulkanRenderingEngine::CreateVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices)
	{
		VulkanBuffer* staging_buffer = nullptr;
		VulkanBuffer* vertex_buffer	 = nullptr;

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

		staging_buffer = this->CreateVulkanBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0
		);
		vertex_buffer = this->CreateVulkanBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			0
		);

		vkMapMemory(
			this->device_manager->GetLogicalDevice(),
			staging_buffer->allocation_info.deviceMemory,
			staging_buffer->allocation_info.offset,
			staging_buffer->allocation_info.size,
			0,
			&staging_buffer->mapped_data
		);
		memcpy(staging_buffer->mapped_data, vertices.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(this->device_manager->GetLogicalDevice(), staging_buffer->allocation_info.deviceMemory);

		this->BufferVulkanTransferCopy(staging_buffer, vertex_buffer, buffer_size);

		this->CleanupVulkanBuffer(staging_buffer);

		return vertex_buffer;
	}

	void VulkanRenderingEngine::CreateVulkanUniformBuffers(VulkanPipeline* pipeline)
	{
		VkDeviceSize buffer_size = sizeof(glm::mat4);

		pipeline->uniform_buffers.resize(VulkanRenderingEngine::max_frames_in_flight);

		for (size_t i = 0; i < VulkanRenderingEngine::max_frames_in_flight; i++)
		{
			pipeline->uniform_buffers[i] = this->CreateVulkanBuffer(
				buffer_size,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				0
			);
		}
	}

	VulkanBuffer* VulkanRenderingEngine::CreateVulkanBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VmaAllocationCreateFlags vmaAllocFlags
	)
	{
		auto* new_buffer = new VulkanBuffer();

		new_buffer->buffer_size = size;

		// Create a buffer handle
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size		= size;
		buffer_info.usage		= usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage				   = VMA_MEMORY_USAGE_AUTO;
		// VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vma_alloc_info.flags				   = vmaAllocFlags;
		vma_alloc_info.requiredFlags		   = properties;

		// if (vkCreateBuffer(this->vkLogicalDevice, &bufferInfo, nullptr, &(new_buffer->buffer)) != VK_SUCCESS)
		if (vmaCreateBuffer(
				this->vma_allocator,
				&buffer_info,
				&vma_alloc_info,
				&(new_buffer->buffer),
				&(new_buffer->allocation),
				&(new_buffer->allocation_info)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating buffer");
			throw std::runtime_error("failed to create buffer!");
		}

		vmaSetAllocationName(this->vma_allocator, new_buffer->allocation, "VulkanBuffer");

		return new_buffer;
	}

	VulkanBuffer* VulkanRenderingEngine::CreateVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize)
	{
		VulkanBuffer* staging_buffer;

		staging_buffer = this->CreateVulkanBuffer(
			dataSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0
		);

		vkMapMemory(
			this->device_manager->GetLogicalDevice(),
			staging_buffer->allocation_info.deviceMemory,
			staging_buffer->allocation_info.offset,
			staging_buffer->allocation_info.size,
			0,
			&staging_buffer->mapped_data
		);

		memcpy(staging_buffer->mapped_data, data, static_cast<size_t>(dataSize));

		vkUnmapMemory(this->device_manager->GetLogicalDevice(), staging_buffer->allocation_info.deviceMemory);

		return staging_buffer;
	}

	void VulkanRenderingEngine::BufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size)
	{
		VkCommandBuffer command_buffer = this->BeginSingleTimeCommandBuffer();

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0; // Optional
		copy_region.dstOffset = 0; // Optional
		copy_region.size	  = size;
		vkCmdCopyBuffer(command_buffer, src->buffer, dest->buffer, 1, &copy_region);

		this->EndSingleTimeCommandBuffer(command_buffer);
	}

	void VulkanRenderingEngine::CleanupVulkanBuffer(VulkanBuffer* buffer)
	{
		// this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up vulkan buffer");

		vkDestroyBuffer(this->device_manager->GetLogicalDevice(), buffer->buffer, nullptr);
		vmaFreeMemory(this->vma_allocator, buffer->allocation);

		// Also delete as we use pointers
		delete buffer;
	}

	// Descriptor sets

	void VulkanRenderingEngine::CreateVulkanDescriptorSetLayout(
		VulkanPipeline* pipeline,
		std::vector<VulkanDescriptorLayoutSettings>& settings
	)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings	= {};
		std::vector<VkDescriptorBindingFlags> binding_flags = {};
		// TODO: Range based for loop?
		for (size_t i = 0; i < settings.size(); i++)
		{
			VkDescriptorSetLayoutBinding new_binding{};
			new_binding.binding			   = settings[i].binding;
			new_binding.descriptorCount	   = settings[i].descriptor_count;
			new_binding.descriptorType	   = settings[i].types;
			new_binding.stageFlags		   = settings[i].stage_flags;
			new_binding.pImmutableSamplers = nullptr;

			bindings.push_back(new_binding);

			VkDescriptorBindingFlags new_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			binding_flags.push_back(new_flags);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info{};
		layout_flags_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layout_flags_info.bindingCount	= static_cast<uint32_t>(binding_flags.size());
		layout_flags_info.pBindingFlags = binding_flags.data();

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings	 = bindings.data();
		layout_info.pNext		 = &layout_flags_info;

		if (vkCreateDescriptorSetLayout(
				this->device_manager->GetLogicalDevice(),
				&layout_info,
				nullptr,
				&(pipeline->vk_descriptor_set_layout)
			) != VK_SUCCESS)
		{
			// TODO: Remove this?
			this->logger->SimpleLog(
				Logging::LogLevel::Error,
				LOGPFX_CURRENT "Vulkan failed to create descriptor set layout"
			);
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanRenderingEngine::CreateVulkanDescriptorPool(
		VulkanPipeline* pipeline,
		std::vector<VulkanDescriptorLayoutSettings>& settings
	)
	{
		std::vector<VkDescriptorPoolSize> pool_sizes{};

		pool_sizes.resize(settings.size());
		for (size_t i = 0; i < settings.size(); i++)
		{
			VkDescriptorPoolSize pool_size{};
			pool_size.type			  = settings[i].types;
			pool_size.descriptorCount = static_cast<uint32_t>(VulkanRenderingEngine::max_frames_in_flight) *
										settings[i].descriptor_count;

			pool_sizes[i] = pool_size;
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes	= pool_sizes.data();
		pool_info.maxSets		= static_cast<uint32_t>(VulkanRenderingEngine::max_frames_in_flight);

		if (vkCreateDescriptorPool(
				this->device_manager->GetLogicalDevice(),
				&pool_info,
				nullptr,
				&(pipeline->vk_descriptor_pool)
			) != VK_SUCCESS)
		{
			// TODO: Remove this?
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed to create descriptor pool");
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	void VulkanRenderingEngine::CreateVulkanDescriptorSets(VulkanPipeline* pipeline)
	{
		std::vector<VkDescriptorSetLayout> layouts(
			VulkanRenderingEngine::max_frames_in_flight,
			pipeline->vk_descriptor_set_layout
		);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool	  = pipeline->vk_descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(VulkanRenderingEngine::max_frames_in_flight);
		alloc_info.pSetLayouts		  = layouts.data();

		pipeline->vk_descriptor_sets.resize(VulkanRenderingEngine::max_frames_in_flight);
		VkResult create_result{};
		if ((create_result = vkAllocateDescriptorSets(
				 this->device_manager->GetLogicalDevice(),
				 &alloc_info,
				 pipeline->vk_descriptor_sets.data()
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

	void VulkanRenderingEngine::CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer command_buffer = this->BeginSingleTimeCommandBuffer();

		VkBufferImageCopy region{};
		region.bufferOffset		 = 0;
		region.bufferRowLength	 = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel	   = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount	   = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		this->EndSingleTimeCommandBuffer(command_buffer);
	}

	void VulkanRenderingEngine::AppendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage)
	{
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType	   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = teximage->use_filter;
		sampler_info.minFilter = teximage->use_filter; // VK_FILTER_LINEAR;

		sampler_info.addressModeU = teximage->use_address_mode; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = teximage->use_address_mode;
		sampler_info.addressModeW = teximage->use_address_mode;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(this->device_manager->GetPhysicalDevice(), &properties);

		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy	  = properties.limits.maxSamplerAnisotropy;

		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		sampler_info.unnormalizedCoordinates = VK_FALSE;

		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp	   = VK_COMPARE_OP_ALWAYS;

		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod		= 0.0f;
		sampler_info.maxLod		= 0.0f;

		if (vkCreateSampler(
				this->device_manager->GetLogicalDevice(),
				&sampler_info,
				nullptr,
				&(teximage->texture_sampler)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Failed to create texture sampler");
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void VulkanRenderingEngine::CleanupVulkanImage(VulkanImage* image)
	{
		vkDestroyImageView(this->device_manager->GetLogicalDevice(), image->image_view, nullptr);
		vkDestroyImage(this->device_manager->GetLogicalDevice(), image->image, nullptr);
		vmaFreeMemory(this->vma_allocator, image->allocation);

		// Also delete as we use pointers
		delete image;
	}

	void VulkanRenderingEngine::CleanupVulkanTextureImage(VulkanTextureImage* image)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up Vulkan texture image");

		vkDestroySampler(this->device_manager->GetLogicalDevice(), image->texture_sampler, nullptr);

		this->CleanupVulkanImage(image->image);

		// Also delete as we use pointers
		delete image;
	}

} // namespace Engine::Rendering
