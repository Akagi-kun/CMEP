#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "InternalEngineObject.hpp"
#include "VBuffer.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Engine
{
	class Engine;
}

namespace Engine::Rendering::Vulkan
{
	enum class VulkanTopologySelection : uint8_t
	{
		VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST,
		VULKAN_RENDERING_ENGINE_TOPOLOGY_LINE_LIST
	};

	struct VSyncObjects
	{
		VkSemaphore image_available;
		VkSemaphore present_ready; // render_finished_semaphores
		VkFence in_flight;
	};

	class VulkanRenderingEngine : public InternalEngineObject
	{
	private:
		// Maximum number of frames in rotation/flight
		static constexpr uint16_t max_frames_in_flight = 3;

		GLFWwindow* window = nullptr;
		ScreenSize window_size;
		std::string window_title;

		uint32_t current_frame	 = 0;
		bool framebuffer_resized = false;

		// Swap chain data
		VSwapchain* swapchain = nullptr;

		// Framebuffers
		std::vector<VkFramebuffer> vk_swap_chain_framebuffers;
		// Multisampling
		VImage* multisampled_color_image = nullptr;
		// Depth buffers
		VImage* vk_depth_buffer			 = nullptr;

		// Command pools and buffers
		std::array<VCommandBuffer*, max_frames_in_flight> command_buffers;

		// Synchronisation
		std::array<VSyncObjects, max_frames_in_flight> sync_objects;

		// Default pipeline
		// VulkanPipeline* graphics_pipeline_default = nullptr;
		VkRenderPass vk_render_pass = VK_NULL_HANDLE;

		// Device manager
		std::shared_ptr<VDeviceManager> device_manager;

		// Memory management
		// VmaAllocator vma_allocator;

		// External callback for rendering
		std::function<void(VkCommandBuffer, uint32_t, Engine*)> external_callback;

		// VkFormat functions
		VkFormat FindVulkanSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);
		VkFormat FindVulkanSupportedDepthFormat();
		bool DoesVulkanFormatHaveStencilComponent(VkFormat format);

		// Swap chain functions
		VkExtent2D ChooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void RecreateVulkanSwapChain();
		void CleanupVulkanSwapChain();

		// Command buffer functions
		void RecordVulkanCommandBuffer(VCommandBuffer* command_buffer, uint32_t image_index);

		// Init functions
		void CreateVulkanSwapChain();
		void CreateVulkanRenderPass();
		void CreateVulkanFramebuffers();
		void CreateVulkanSyncObjects();
		void CreateVulkanDepthResources();
		void CreateMultisampledColorResources();

	public:
		VulkanRenderingEngine(Engine* with_engine, ScreenSize with_window_size, std::string title);
		~VulkanRenderingEngine();

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW(ScreenSize with_size);

		// Cleanup functions
		void Cleanup();

		// Engine functions
		void DrawFrame();
		void SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Engine*)> callback);

		// Buffer functions
		VBuffer* CreateVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices);
		VBuffer* CreateVulkanStagingBufferWithData(void* data, VkDeviceSize data_size);

		// Image functions
		void CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		// Pipeline functions
		VulkanPipelineSettings GetVulkanDefaultPipelineSettings();
		VPipeline* CreateVulkanPipeline(VulkanPipelineSettings& settings);

		// Getters
		[[nodiscard]] std::weak_ptr<VDeviceManager> GetDeviceManager()
		{
			return this->device_manager;
		}

		[[nodiscard]] GLFWwindowData GetWindow() const;

		[[nodiscard]] static uint32_t GetMaxFramesInFlight()
		{
			return VulkanRenderingEngine::max_frames_in_flight;
		}

		void SyncDeviceWaitIdle();

		// Utility functions
		uint32_t FindVulkanMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
	};
} // namespace Engine::Rendering::Vulkan
