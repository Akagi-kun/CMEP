#pragma once

#include "Rendering/Transform.hpp"

#include "ImportVulkan.hpp"
#include "InternalEngineObject.hpp"
#include "VulkanDeviceManager.hpp"
#include "VulkanStructDefs.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Engine
{
	class Engine;
}

namespace Engine::Rendering
{
	enum VulkanTopologySelection
	{
		VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST,
		VULKAN_RENDERING_ENGINE_TOPOLOGY_LINE_LIST
	};

	class VulkanRenderingEngine : public InternalEngineObject
	{
	private:
		const uint_fast16_t max_frames_in_flight = 2;

		GLFWwindow* window = nullptr;
		ScreenSize window_size;
		// int_fast16_t window_x = 0, window_y = 0;
		std::string window_title;

		static std::vector<char> ReadShaderFile(const std::string& path);

		uint32_t current_frame	 = 0;
		bool framebuffer_resized = false;

		// Swap chains
		VkSwapchainKHR vk_swap_chain = VK_NULL_HANDLE;
		std::vector<VkImage> vk_swap_chain_images;
		VkFormat vk_swap_chain_image_format{};
		VkExtent2D vk_swap_chain_extent{};
		std::vector<VkImageView> vk_swap_chain_image_views;

		// Multisampling
		VulkanImage* multisampled_color_image{};

		// Framebuffers
		std::vector<VkFramebuffer> vk_swap_chain_framebuffers;

		// Command pools and buffers
		VkCommandPool vk_command_pool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> vk_command_buffers;

		// Synchronisation
		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;

		// Pipeline
		VulkanPipeline* graphics_pipeline_default = nullptr;
		VkRenderPass vk_render_pass				  = VK_NULL_HANDLE;

		// Depth buffers
		VulkanImage* vk_depth_buffer = nullptr;

		// Device manager
		std::unique_ptr<VulkanDeviceManager> device_manager;

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
		void CreateVulkanSwapChainViews();
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
		void CreateVulkanCommandPools();
		void CreateVulkanCommandBuffers();
		void CreateVulkanSyncObjects();
		void CreateVulkanDepthResources();
		void CreateMultisampledColorResources();
		void CreateVulkanMemoryAllocator();

	public:
		VulkanRenderingEngine() = default;

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW(ScreenSize with_size);

		// Cleanup functions
		void Cleanup();
		void CleanupVulkanBuffer(VulkanBuffer* buffer);
		void CleanupVulkanTextureImage(VulkanTextureImage* image);
		void CleanupVulkanPipeline(VulkanPipeline* pipeline);
		void CleanupVulkanImage(VulkanImage* image);

		// Init
		void Init(unsigned int xsize, unsigned int ysize, std::string title);
		void PrepRun();

		// Engine functions
		void DrawFrame();
		void SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Engine*)> callback);

		// Buffer functions
		VulkanBuffer* CreateVulkanBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VmaAllocationCreateFlags vmaAllocFlags
		);
		void BufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size);
		VulkanBuffer* CreateVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices);
		VulkanBuffer* CreateVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize);

		// Image functions
		void CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void AppendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);

		// Command buffer functions
		VkCommandBuffer BeginVulkanSingleTimeCommandsCommandBuffer();
		void EndVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer);

		// Pipeline functions
		VulkanPipelineSettings GetVulkanDefaultPipelineSettings();
		VulkanPipeline* CreateVulkanPipelineFromPrealloc(
			VulkanPipeline* pipeline,
			VulkanPipelineSettings& settings,
			const std::string& vert_path,
			const std::string& frag_path
		);
		VulkanPipeline* CreateVulkanPipeline(
			VulkanPipelineSettings& settings,
			const std::string& vert_path,
			const std::string& frag_path
		);

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
		VkDevice GetLogicalDevice();
		[[nodiscard]] GLFWwindowData GetWindow() const;
		[[nodiscard]] uint32_t GetMaxFramesInFlight() const;
		VmaAllocator GetVMAAllocator();

		// Utility functions
		uint32_t FindVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
} // namespace Engine::Rendering
