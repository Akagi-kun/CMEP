#pragma once

#include "ImportVulkan.hpp"
#include "InternalEngineObject.hpp"
#include "VulkanDeviceManager.hpp"
#include "VulkanStructDefs.hpp"

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

	extern VKAPI_ATTR VkBool32 VKAPI_CALL vulcanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	extern VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	);

	class VulkanRenderingEngine : public InternalEngineObject
	{
	private:
		const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

		GLFWwindow* window = nullptr;
		int_fast16_t window_x = 0, window_y = 0;
		std::string window_title;

		static std::vector<char> ReadShaderFile(std::string path);

		uint32_t currentFrame = 0;
		bool framebufferResized = false;

		// Swap chains
		VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> vkSwapChainImages{};
		VkFormat vkSwapChainImageFormat{};
		VkExtent2D vkSwapChainExtent{};
		std::vector<VkImageView> vkSwapChainImageViews{};

		// Multisampling
		VulkanImage* multisampledColorImage{};

		// Framebuffers
		std::vector<VkFramebuffer> vkSwapChainFramebuffers{};

		// Command pools and buffers
		VkCommandPool vkCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> vkCommandBuffers{};

		// Synchronisation
		std::vector<VkSemaphore> imageAvailableSemaphores{};
		std::vector<VkSemaphore> renderFinishedSemaphores{};
		std::vector<VkFence> inFlightFences{};

		// Pipeline
		VulkanPipeline* graphicsPipelineDefault = nullptr;
		VkRenderPass vkRenderPass = VK_NULL_HANDLE;

		// Depth buffers
		VulkanImage* vkDepthBuffer = nullptr;

		// Device manager
		std::unique_ptr<VulkanDeviceManager> deviceManager{};

		// Memory management
		VmaAllocator vmaAllocator;

		// External callback for rendering
		std::function<void(VkCommandBuffer, uint32_t, Engine*)> external_callback;

		// VkFormat functions
		VkFormat findVulkanSupportedFormat(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features
		);
		VkFormat findVulkanSupportedDepthFormat();
		bool doesVulkanFormatHaveStencilComponent(VkFormat format);

		// Swap chain functions
		VkExtent2D chooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createVulkanSwapChainViews();
		void recreateVulkanSwapChain();
		void cleanupVulkanSwapChain();

		// Command buffer functions
		void recordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		// Shaders functions
		VkShaderModule createVulkanShaderModule(const std::vector<char>& code);

		// Init functions
		void createVulkanSwapChain();
		void createVulkanDefaultGraphicsPipeline();
		void createVulkanRenderPass();
		void createVulkanFramebuffers();
		void createVulkanCommandPools();
		void createVulkanCommandBuffers();
		void createVulkanSyncObjects();
		void createVulkanDepthResources();
		void createMultisampledColorResources();
		void createVulkanMemoryAllocator();

	public:
		VulkanRenderingEngine()
		{
		}

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW(int width, int height);

		// Cleanup functions
		void cleanup();
		void cleanupVulkanBuffer(VulkanBuffer* buffer);
		void cleanupVulkanTextureImage(VulkanTextureImage* buffer);
		void cleanupVulkanPipeline(VulkanPipeline* pipeline);
		void cleanupVulkanImage(VulkanImage* image);

		// Init
		void init(unsigned int xsize, unsigned int ysize, std::string title);
		void prepRun();

		// Engine functions
		void drawFrame();
		void SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Engine*)> callback);

		// Buffer functions
		VulkanBuffer* createVulkanBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VmaAllocationCreateFlags vmaAllocFlags
		);
		void bufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size);
		VulkanBuffer* createVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices);
		VulkanBuffer* createVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize);

		// Image functions
		void copyVulcanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);

		// Command buffer functions
		VkCommandBuffer beginVulkanSingleTimeCommandsCommandBuffer();
		void endVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer);

		// Pipeline functions
		VulkanPipelineSettings getVulkanDefaultPipelineSettings();
		VulkanPipeline* createVulkanPipelineFromPrealloc(
			VulkanPipeline* pipeline, VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path
		);
		VulkanPipeline* createVulkanPipeline(
			VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path
		);

		// Pipeline descriptor functions
		void createVulkanDescriptorSetLayout(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);
		void createVulkanUniformBuffers(VulkanPipeline* pipeline);
		void createVulkanDescriptorPool(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);
		void createVulkanDescriptorSets(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);

		// Getters
		VkDevice GetLogicalDevice();
		GLFWwindowData const GetWindow();
		uint32_t GetMaxFramesInFlight();
		VmaAllocator GetVMAAllocator();

		// Utility functions
		uint32_t findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
} // namespace Engine::Rendering
