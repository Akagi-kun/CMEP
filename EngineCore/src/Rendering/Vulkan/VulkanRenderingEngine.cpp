#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Rendering/Vulkan/DeviceManager.hpp"
#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/Wrappers/Buffer.hpp"
#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/CommandPool.hpp" // IWYU pragma: keep
#include "Rendering/Vulkan/Wrappers/Image.hpp"		 // IWYU pragma: keep
#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"
#include "Rendering/Vulkan/Wrappers/Window.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine::Rendering::Vulkan
{
	[[noreturn]] static void GlfwErrorCallback(int error, const char* description)
	{
		using namespace std::string_literals;

		throw std::runtime_error("GLFW error handler callback called! Code: '"s.append(std::to_string(error))
									 .append("'; description: '")
									 .append(description)
									 .append("'"));
	}

	////////////////////////////////////////////////////////////////////////
	///////////////////////    Runtime functions    ////////////////////////
	////////////////////////////////////////////////////////////////////////
#pragma region Runtime functions

	void VulkanRenderingEngine::RecordFrameRenderCommands(CommandBuffer* command_buffer, uint32_t image_index)
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
		glfwGetFramebufferSize(this->window->native_handle, &width, &height);

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
#pragma region Init functions

	VulkanRenderingEngine::VulkanRenderingEngine(Engine* with_engine, ScreenSize with_window_size, std::string title)
		: InternalEngineObject(with_engine)
	{
		// Initialize GLFW
		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("GLFW returned GLFW_FALSE on glfwInit!");
		}
		glfwSetErrorCallback(GlfwErrorCallback);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "GLFW initialized");

		// Create a GLFW window
		this->window = new Window(
			with_window_size,
			std::move(title),
			{
				{GLFW_VISIBLE, GLFW_FALSE},
				{GLFW_RESIZABLE, GLFW_FALSE},
			}
		);

		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "%u vulkan extensions supported", extension_count);

		this->device_manager = std::make_shared<DeviceManager>(this->logger, this->window);

		this->CreateVulkanSwapChain();

		// Create command buffers
		for (auto& vk_command_buffer : this->command_buffers)
		{
			vk_command_buffer = this->device_manager->GetCommandPool()->AllocateCommandBuffer();
		}

		this->CreateVulkanSyncObjects();
	}

	VulkanRenderingEngine::~VulkanRenderingEngine()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		vkDeviceWaitIdle(logical_device);

		this->CleanupVulkanSwapChain();
		this->CleanupVulkanSyncObjects();

		for (auto& vk_command_buffer : this->command_buffers)
		{
			delete vk_command_buffer;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up default vulkan pipeline");

		// Destroy device
		this->device_manager.reset();

		// Clean up GLFW
		delete window;
		glfwTerminate();
	}

	void VulkanRenderingEngine::CreateVulkanSwapChain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swap_chain_support = this->device_manager->QuerySwapChainSupport();

		VkExtent2D extent = this->ChooseVulkanSwapExtent(swap_chain_support.capabilities);

		// Request one image more than is the required minimum
		// uint32_t swapchain_image_count = swap_chain_support.capabilities.minImageCount + 1;
		// Temporary fix for screen lag
		// uint32_t swapchain_image_count = 1;
		uint32_t swapchain_image_count = VulkanRenderingEngine::max_frames_in_flight;

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

		this->swapchain = new Swapchain(this->device_manager.get(), extent, swapchain_image_count);

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Vulkan swap chain created");
	}

	void VulkanRenderingEngine::RecreateVulkanSwapChain()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		// If window is minimized, wait for it to show up again
		// int width			   = 0;
		// int height			   = 0;
		ScreenSize framebuffer = this->window->GetFramebufferSize();
		// glfwGetFramebufferSize(this->window->native_handle, &width, &height);
		// while (width == 0 || height == 0)
		while (framebuffer.x == 0 || framebuffer.y == 0)
		{
			framebuffer = this->window->GetFramebufferSize();
			// glfwGetFramebufferSize(this->window->native_handle, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logical_device);

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Recreating vulkan swap chain");

		// Clean up old swap chain
		this->CleanupVulkanSwapChain();
		this->CleanupVulkanSyncObjects();

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Old swap chain cleaned up");

		// Create a new swap chain
		this->CreateVulkanSwapChain();
		this->CreateVulkanSyncObjects();
	}

	void VulkanRenderingEngine::CleanupVulkanSwapChain()
	{
		delete this->swapchain;

		this->swapchain = nullptr;
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
				vkCreateFence(logical_device, &fence_info, nullptr, &(sync_object_frame.in_flight)) != VK_SUCCESS)
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating sync objects");
				throw std::runtime_error("failed to create sync objects!");
			}
		}
	}

	void VulkanRenderingEngine::CleanupVulkanSyncObjects()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		for (auto& sync_object : this->sync_objects)
		{
			vkDestroySemaphore(logical_device, sync_object.present_ready, nullptr);
			vkDestroySemaphore(logical_device, sync_object.image_available, nullptr);
			vkDestroyFence(logical_device, sync_object.in_flight, nullptr);
		}
	}
#pragma endregion
	////////////////////////////////////////////////////////////////////////
	///////////////////////    Public Interface    /////////////////////////
	////////////////////////////////////////////////////////////////////////

#pragma region Public interface
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
		if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || this->window->is_resized ||
			acquire_result == VK_SUBOPTIMAL_KHR)
		{
			// Increment current_frame (clamp to max_frames_in_flight)
			// this->current_frame		  = (this->current_frame + 1) % this->max_frames_in_flight;
			this->window->is_resized = false;

			this->logger->SimpleLog(
				Logging::LogLevel::Warning,
				"acquire_result was VK_ERROR_OUT_OF_DATE_KHR, VK_SUBOPTIMAL_KHR, or framebuffer was resized!"
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
		this->RecordFrameRenderCommands(this->command_buffers[this->current_frame], image_index);

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
		std::function<void(Vulkan::CommandBuffer*, uint32_t, Engine*)> callback
	)
	{
		this->external_callback = std::move(callback);
	}

	void VulkanRenderingEngine::SyncDeviceWaitIdle()
	{
		vkDeviceWaitIdle(this->device_manager->GetLogicalDevice());
	}

	/* void VulkanRenderingEngine::SignalFramebufferResizeGLFW(ScreenSize with_size)
	{
		this->framebuffer_resized = true;
		this->window->size		  = with_size;
	} */

	// Buffers

	Buffer* VulkanRenderingEngine::CreateVulkanVertexBufferFromData(const std::vector<RenderingVertex>& vertices)
	{
		CommandBuffer* command_buffer = this->device_manager->GetCommandPool()->AllocateCommandBuffer();

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

		auto* staging_buffer = this->CreateVulkanStagingBufferWithData(vertices.data(), buffer_size);

		auto* vertex_buffer = new Buffer(
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

	Buffer* VulkanRenderingEngine::CreateVulkanStagingBufferWithData(const void* data, VkDeviceSize data_size)
	{
		Buffer* staging_buffer;

		staging_buffer = new Buffer(
			this->device_manager.get(),
			data_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		staging_buffer->MemoryCopy(data, data_size);

		return staging_buffer;
	}
#pragma endregion

} // namespace Engine::Rendering::Vulkan
