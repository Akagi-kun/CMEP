#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/Wrappers/Buffer.hpp"
#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/CommandPool.hpp" // IWYU pragma: keep
#include "Rendering/Vulkan/Wrappers/Image.hpp"		 // IWYU pragma: keep
#include "Rendering/Vulkan/Wrappers/Instance.hpp"
#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"
#include "Rendering/Vulkan/Wrappers/Window.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine::Rendering::Vulkan
{
	////////////////////////////////////////////////////////////////////////
	///////////////////////    Runtime functions    ////////////////////////
	////////////////////////////////////////////////////////////////////////
#pragma region Runtime functions

	void VulkanRenderingEngine::RecordFrameRenderCommands(CommandBuffer* command_buffer, uint32_t image_index)
	{
		command_buffer->BeginCmdBuffer(0);

		auto* swapchain = this->instance->GetWindow()->GetSwapchain();

		swapchain->BeginRenderPass(command_buffer, image_index);

		VkViewport viewport{};
		viewport.x		  = 0.0f;
		viewport.y		  = 0.0f;
		viewport.width	  = static_cast<float>(swapchain->GetExtent().width);
		viewport.height	  = static_cast<float>(swapchain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(*command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapchain->GetExtent();
		vkCmdSetScissor(*command_buffer, 0, 1, &scissor);

		// Perform actual render
		if (this->external_callback)
		{
			this->external_callback(command_buffer, current_frame, this->owner_engine);
		}

		command_buffer->EndRenderPass();

		command_buffer->EndCmdBuffer();
	}

	/* uint32_t VulkanRenderingEngine::FindVulkanMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(this->GetDeviceManager()->GetPhysicalDevice(), &mem_properties);

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
	} */

#pragma endregion
	////////////////////////////////////////////////////////////////////////
	////////////////////////    Init functions    //////////////////////////
	////////////////////////////////////////////////////////////////////////
#pragma region Init functions

	VulkanRenderingEngine::VulkanRenderingEngine(
		Engine* with_engine,
		ScreenSize with_window_size,
		const std::string& with_title
	)
		: InternalEngineObject(with_engine)
	{
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "%u vulkan extensions supported", extension_count);

		this->instance = new Instance(
			this->logger,
			{
				with_window_size,
				with_title,
				{
					{GLFW_VISIBLE, GLFW_FALSE},
					{GLFW_RESIZABLE, GLFW_TRUE},
				},
			}
		);
	}

	VulkanRenderingEngine::~VulkanRenderingEngine()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		logical_device->WaitDeviceIdle();
		// vkDeviceWaitIdle(logical_device);

		// this->CleanupVulkanSwapChain();
		//  this->CleanupVulkanSyncObjects();

		/* for (auto& vk_command_buffer : this->command_buffers)
		{
			delete vk_command_buffer;
		} */

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Cleaning up default vulkan pipeline");

		// Destroy device
		// this->device_manager.reset();

		delete instance;

		// Clean up GLFW
		// delete window;
		glfwTerminate();
	}

#pragma endregion
	////////////////////////////////////////////////////////////////////////
	///////////////////////    Public Interface    /////////////////////////
	////////////////////////////////////////////////////////////////////////

#pragma region Public interface
	void VulkanRenderingEngine::DrawFrame()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		auto* swapchain = this->instance->GetWindow()->GetSwapchain();

		auto& render_target = swapchain->GetRenderTarget(this->current_frame);
		// auto& frame_sync_objects = this->sync_objects[this->current_frame];

		// Wait for fence
		vkWaitForFences(*logical_device, 1, &render_target.sync_objects.in_flight, VK_TRUE, UINT64_MAX);

		// Reset fence after wait is over
		// (fence has to be reset before being used again)
		vkResetFences(*logical_device, 1, &render_target.sync_objects.in_flight);

		// Index of framebuffer in this->vk_swap_chain_framebuffers
		uint32_t image_index;
		// Acquire render target
		// the render target is an image in the swap chain
		VkResult acquire_result = vkAcquireNextImageKHR(
			*logical_device,
			*swapchain,
			UINT64_MAX,
			render_target.sync_objects.image_available,
			nullptr,
			&image_index
		);

		// If it's necessary to recreate a swap chain
		if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || this->GetWindow()->status.is_resized ||
			acquire_result == VK_SUBOPTIMAL_KHR)
		{
			// Increment current_frame (clamp to max_frames_in_flight)
			// this->current_frame		  = (this->current_frame + 1) % this->max_frames_in_flight;
			this->GetWindow()->status.is_resized = false;

			this->logger->SimpleLog(
				Logging::LogLevel::Warning,
				"acquire_result was VK_ERROR_OUT_OF_DATE_KHR, VK_SUBOPTIMAL_KHR, or framebuffer was resized!"
			);

			// this->RecreateVulkanSwapChain();

			return;
		}

		if (acquire_result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		// Reset command buffer to initial state´
		render_target.command_buffer->ResetBuffer();

		// Records render into command buffer
		this->RecordFrameRenderCommands(render_target.command_buffer, image_index);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::array<VkCommandBuffer, 1> command_buffers = {*render_target.command_buffer};

		// Wait semaphores
		VkSemaphore wait_semaphores[]	   = {render_target.sync_objects.image_available};
		VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount	   = 1;
		submit_info.pWaitSemaphores		   = wait_semaphores;
		submit_info.pWaitDstStageMask	   = wait_stages;
		submit_info.commandBufferCount	   = static_cast<uint32_t>(command_buffers.size());
		submit_info.pCommandBuffers		   = command_buffers.data();

		// Signal semaphores to be signaled once
		// all submit_info.pCommandBuffers finish executing
		VkSemaphore signal_semaphores[]	 = {render_target.sync_objects.present_ready};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores	 = signal_semaphores;

		// Submit to queue
		// passed fence will be signaled when command buffer execution is finished
		if (vkQueueSubmit(logical_device->GetGraphicsQueue(), 1, &submit_info, render_target.sync_objects.in_flight) !=
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

		VkSwapchainKHR swap_chains[] = {*swapchain};
		present_info.swapchainCount	 = 1;
		present_info.pSwapchains	 = swap_chains;
		present_info.pImageIndices	 = &image_index;
		present_info.pResults		 = nullptr; // Optional

		// Present current image to the screen
		if (vkQueuePresentKHR(logical_device->GetPresentQueue(), &present_info) != VK_SUCCESS)
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

	// Buffers

	Buffer* VulkanRenderingEngine::CreateVulkanVertexBufferFromData(const std::vector<RenderingVertex>& vertices)
	{
		CommandBuffer* command_buffer = instance->GetCommandPool()->AllocateCommandBuffer();

		VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

		auto* staging_buffer = this->CreateVulkanStagingBufferWithData(vertices.data(), buffer_size);

		auto* vertex_buffer = new Buffer(
			instance,
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
			instance,
			data_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		staging_buffer->MemoryCopy(data, data_size);

		return staging_buffer;
	}
#pragma endregion

} // namespace Engine::Rendering::Vulkan
