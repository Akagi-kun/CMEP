#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/VImage.hpp"
#include "Rendering/Vulkan/Wrappers/VSwapchain.hpp"

#include "Logging/Logging.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

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

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Vulkan swap chain created");
	}

	void VulkanRenderingEngine::RecreateVulkanSwapChain()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		// If window is minimized, wait for it to show up again
		int width  = 0;
		int height = 0;
		glfwGetFramebufferSize(this->window.native_handle, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(this->window.native_handle, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logical_device);

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Recreating vulkan swap chain");

		// Clean up old swap chain
		this->CleanupVulkanSwapChain();

		// Create a new swap chain
		this->CreateVulkanSwapChain();
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

} // namespace Engine::Rendering::Vulkan
