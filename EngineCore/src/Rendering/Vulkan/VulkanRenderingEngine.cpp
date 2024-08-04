#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/VBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/VCommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/VCommandPool.hpp" // IWYU pragma: keep
#include "Rendering/Vulkan/Wrappers/VImage.hpp"		  // IWYU pragma: keep
#include "Rendering/Vulkan/Wrappers/VSwapchain.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering::Vulkan
{
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto* app = reinterpret_cast<VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
		app->SignalFramebufferResizeGLFW({static_cast<uint_fast16_t>(width), static_cast<uint_fast16_t>(height)});
	}

	////////////////////////////////////////////////////////////////////////
	///////////////////////    Runtime functions    ////////////////////////
	////////////////////////////////////////////////////////////////////////
#pragma region Runtime functions

	void VulkanRenderingEngine::RecordVulkanCommandBuffer(VCommandBuffer* command_buffer, uint32_t image_index)
	{
		command_buffer->BeginCmdBuffer(0);

		auto& command_buf_handle = command_buffer->GetNativeHandle();

		this->swapchain->BeginRenderPass(command_buffer, image_index);

		VkViewport viewport{};
		viewport.x		  = 0.0f;
		viewport.y		  = 0.0f;
		viewport.width	  = static_cast<float>(this->swapchain->GetExtent().width);
		viewport.height	  = static_cast<float>(this->swapchain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buf_handle, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = this->swapchain->GetExtent();
		vkCmdSetScissor(command_buf_handle, 0, 1, &scissor);

		// Perform actual render
		if (this->external_callback)
		{
			this->external_callback(command_buffer, current_frame, this->owner_engine);
		}

		command_buffer->EndRenderPass();

		command_buffer->EndCmdBuffer();
	}

	VkExtent2D VulkanRenderingEngine::ChooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int width;
		int height;
		glfwGetFramebufferSize(this->window.native_handle, &width, &height);

		VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		actual_extent.width =
			std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height =
			std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}

	uint32_t VulkanRenderingEngine::FindVulkanMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(this->device_manager->GetPhysicalDevice(), &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
			if ((type_filter & (1 << i)) != 0 &&
				(mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to find required memory type");
		throw std::runtime_error("failed to find required memory type!");
	}

	VkFormat VulkanRenderingEngine::FindVulkanSupportedFormat(
		VkPhysicalDevice with_device,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(with_device, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported vkFormat!");
	}

	VkFormat VulkanRenderingEngine::FindVulkanSupportedDepthFormat(VkPhysicalDevice with_device)
	{
		return FindVulkanSupportedFormat(
			with_device,
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

#pragma endregion
	////////////////////////////////////////////////////////////////////////
	////////////////////////    Init functions    //////////////////////////
	////////////////////////////////////////////////////////////////////////

	VulkanRenderingEngine::VulkanRenderingEngine(Engine* with_engine, ScreenSize with_window_size, std::string title)
		: InternalEngineObject(with_engine),
		  window{nullptr, with_window_size, std::move(title)} //, window_title(std::move(title))
	{
		// Initialize GLFW
		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("GLFW returned GLFW_FALSE on glfwInit!");
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

		this->window.native_handle = glfwCreateWindow(
			static_cast<int>(this->window.size.x),
			static_cast<int>(this->window.size.y),
			this->window.title.c_str(),
			nullptr, // glfwGetPrimaryMonitor(),
			nullptr
		);
		glfwSetWindowUserPointer(this->window.native_handle, this);
		glfwSetFramebufferSizeCallback(this->window.native_handle, FramebufferResizeCallback);

		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "%u vulkan extensions supported", extension_count);

		this->device_manager = std::make_shared<VDeviceManager>(this->owner_engine, this->window.native_handle);

		this->CreateVulkanSwapChain();
		// this->CreateVulkanRenderPass();

		// Create command buffers
		for (auto& vk_command_buffer : this->command_buffers)
		{
			vk_command_buffer = this->device_manager->GetCommandPool()->AllocateCommandBuffer();
			// vk_command_buffer = new VCommandBuffer(this->device_manager.get(),
			// this->device_manager->GetCommandPool());
		}

		// this->CreateMultisampledColorResources();
		// this->CreateVulkanDepthResources();
		//  this->CreateVulkanFramebuffers();
		this->CreateVulkanSyncObjects();
	}

	VulkanRenderingEngine::~VulkanRenderingEngine()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		vkDeviceWaitIdle(logical_device);

		this->CleanupVulkanSwapChain();

		// delete this->multisampled_color_image;
		// delete this->vk_depth_buffer;

		for (size_t i = 0; i < VulkanRenderingEngine::max_frames_in_flight; i++)
		{
			vkDestroySemaphore(logical_device, this->sync_objects[i].present_ready, nullptr);
			vkDestroySemaphore(logical_device, this->sync_objects[i].image_available, nullptr);
			vkDestroyFence(logical_device, this->sync_objects[i].in_flight, nullptr);
		}

		for (auto& vk_command_buffer : this->command_buffers)
		{
			delete vk_command_buffer;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up default vulkan pipeline");

		// vkDestroyRenderPass(logical_device, this->vk_render_pass, nullptr);

		// Destroy device
		this->device_manager.reset();

		// Clean up GLFW
		glfwDestroyWindow(this->window.native_handle);
		glfwTerminate();
	}

	// Rest of section moved to 'VulkanRenderingEngine_Init.cpp'
	//

	////////////////////////////////////////////////////////////////////////
	///////////////////////    Public Interface    /////////////////////////
	////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::DrawFrame()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		auto& frame_sync_objects = this->sync_objects[this->current_frame];

		// Wait for fence
		vkWaitForFences(logical_device, 1, &frame_sync_objects.in_flight, VK_TRUE, UINT64_MAX);

		// Reset fence after wait is over
		// (fence has to be reset before being used again)
		vkResetFences(logical_device, 1, &frame_sync_objects.in_flight);

		// Index of framebuffer in this->vk_swap_chain_framebuffers
		uint32_t image_index;
		// Acquire render target
		// the render target is an image in the swap chain
		VkResult acquire_result = vkAcquireNextImageKHR(
			logical_device,
			this->swapchain->GetNativeHandle(),
			UINT64_MAX,
			frame_sync_objects.image_available,
			nullptr,
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
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		// Reset command buffer to initial state
		vkResetCommandBuffer(this->command_buffers[this->current_frame]->GetNativeHandle(), 0);

		// Records render into command buffer
		this->RecordVulkanCommandBuffer(this->command_buffers[this->current_frame], image_index);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Wait semaphores
		VkSemaphore wait_semaphores[]	   = {frame_sync_objects.image_available};
		VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount	   = 1;
		submit_info.pWaitSemaphores		   = wait_semaphores;
		submit_info.pWaitDstStageMask	   = wait_stages;
		submit_info.commandBufferCount	   = 1;
		submit_info.pCommandBuffers		   = &this->command_buffers[this->current_frame]->GetNativeHandle();

		// Signal semaphores to be signaled once
		// all submit_info.pCommandBuffers finish executing
		VkSemaphore signal_semaphores[]	 = {frame_sync_objects.present_ready};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores	 = signal_semaphores;

		// Submit to queue
		// passed fence will be signaled when command buffer execution is finished
		if (vkQueueSubmit(this->device_manager->GetGraphicsQueue(), 1, &submit_info, frame_sync_objects.in_flight) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		// Increment current frame
		// this->current_frame = 0;
		this->current_frame = (this->current_frame + 1) % VulkanRenderingEngine::max_frames_in_flight;

		VkPresentInfoKHR present_info{};
		present_info.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		// Wait for present_ready_semaphores to be signaled
		// (when signaled = image is ready to be presented)
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores	= signal_semaphores;

		VkSwapchainKHR swap_chains[] = {this->swapchain->GetNativeHandle()};
		present_info.swapchainCount	 = 1;
		present_info.pSwapchains	 = swap_chains;
		present_info.pImageIndices	 = &image_index;
		present_info.pResults		 = nullptr; // Optional

		// Present current image to the screen
		if (vkQueuePresentKHR(this->device_manager->GetPresentQueue(), &present_info) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present queue!");
		}
	}

	void VulkanRenderingEngine::SetRenderCallback(
		std::function<void(Vulkan::VCommandBuffer*, uint32_t, Engine*)> callback
	)
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
		this->window.size		  = with_size;
	}

	// Pipelines

	/* VulkanPipelineSettings VulkanRenderingEngine::GetVulkanDefaultPipelineSettings()
	{
		// VkRect2D scissor{};
		// scissor.offset = {0, 0};
		// scissor.extent = this->swapchain->GetExtent();

		VulkanPipelineSettings default_settings{};
		default_settings.extent						= this->swapchain->GetExtent();
		// default_settings.input_topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		// default_settings.scissor					= scissor;
		default_settings.descriptor_layout_settings = {};

		return default_settings;
	} */

	// Buffers

	VBuffer* VulkanRenderingEngine::CreateVulkanVertexBufferFromData(const std::vector<RenderingVertex>& vertices)
	{
		VCommandBuffer* command_buffer = this->device_manager->GetCommandPool()->AllocateCommandBuffer();

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

		auto* staging_buffer = this->CreateVulkanStagingBufferWithData(vertices.data(), buffer_size);

		auto* vertex_buffer = new VBuffer(
			this->device_manager.get(),
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// Copy into final buffer
		command_buffer->BufferBufferCopy(staging_buffer, vertex_buffer, {VkBufferCopy{0, 0, buffer_size}});

		delete staging_buffer;
		delete command_buffer;

		return vertex_buffer;
	}

	VBuffer* VulkanRenderingEngine::CreateVulkanStagingBufferWithData(const void* data, VkDeviceSize data_size)
	{
		VBuffer* staging_buffer;

		staging_buffer = new VBuffer(
			this->device_manager.get(),
			data_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		staging_buffer->MemoryCopy(data, data_size);

		return staging_buffer;
	}

} // namespace Engine::Rendering::Vulkan
