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
		VkFence acquire_ready;	   // maybe useless?
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
		std::array<VCommandBuffer*, max_frames_in_flight> vk_command_buffers;

		// Synchronisation
		// TODO: Struct SyncObjects together into single vector
		// std::array<VkSemaphore, max_frames_in_flight> image_available_semaphores;
		// std::array<VkSemaphore, max_frames_in_flight> present_ready_semaphores; // render_finished_semaphores
		// std::array<VkFence, max_frames_in_flight> acquire_ready_fences;			// maybe useless?
		// std::array<VkFence, max_frames_in_flight> in_flight_fences;

		std::array<VSyncObjects, max_frames_in_flight> sync_objects;

		// Default pipeline
		VulkanPipeline* graphics_pipeline_default = nullptr;
		VkRenderPass vk_render_pass				  = VK_NULL_HANDLE;

		// Device manager
		std::shared_ptr<VulkanDeviceManager> device_manager;

		// Memory management
		VmaAllocator vma_allocator;

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
		void RecordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		// Shaders functions
		VkShaderModule CreateVulkanShaderModule(const std::vector<char>& code);

		// Init functions
		void CreateVulkanSwapChain();
		void CreateVulkanDefaultGraphicsPipeline();
		void CreateVulkanRenderPass();
		void CreateVulkanFramebuffers();
		void CreateVulkanSyncObjects();
		void CreateVulkanDepthResources();
		void CreateMultisampledColorResources();
		void CreateVulkanMemoryAllocator();

	public:
		// VulkanRenderingEngine() = delete;
		using InternalEngineObject::InternalEngineObject;

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW(ScreenSize with_size);

		// Cleanup functions
		void Cleanup();
		// void CleanupVulkanBuffer(VulkanBuffer* buffer);
		void CleanupVulkanPipeline(VulkanPipeline* pipeline);

		// Init
		void Init(unsigned int xsize, unsigned int ysize, std::string title);
		void PrepRun();

		// Engine functions
		void DrawFrame();
		void SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Engine*)> callback);

		// Buffer functions
		VBuffer* CreateVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices);
		VBuffer* CreateVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize);

		// Image functions
		void CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		// Pipeline functions
		VulkanPipelineSettings GetVulkanDefaultPipelineSettings();
		VulkanPipeline* CreateVulkanPipelineFromPrealloc(VulkanPipeline* pipeline, VulkanPipelineSettings& settings);
		VulkanPipeline* CreateVulkanPipeline(VulkanPipelineSettings& settings);

		// Pipeline descriptor functions
		void CreateVulkanDescriptorSetLayout(
			VulkanPipeline* pipeline,
			std::vector<VulkanDescriptorLayoutSettings>& settings
		);
		void CreateVulkanUniformBuffers(VulkanPipeline* pipeline);
		void CreateVulkanDescriptorPool(
			VulkanPipeline* pipeline,
			std::vector<VulkanDescriptorLayoutSettings>& settings
		);
		void CreateVulkanDescriptorSets(VulkanPipeline* pipeline);

		// Getters
		[[nodiscard]] std::weak_ptr<VulkanDeviceManager> GetDeviceManager();
		[[nodiscard]] GLFWwindowData GetWindow() const;
		[[nodiscard]] static uint32_t GetMaxFramesInFlight();
		[[nodiscard]] VmaAllocator GetVMAAllocator();
		[[nodiscard]] VCommandPool* GetCommandPool();

		void SyncDeviceWaitIdle();

		// Utility functions
		uint32_t FindVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
} // namespace Engine::Rendering::Vulkan
