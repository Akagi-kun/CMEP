#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

#include "glm/glm.hpp"

#include <string>
#include <array>
#include <vector>
#include <functional>
#include <optional>
#include <cstring>

namespace Engine::Rendering
{
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct GLFWwindowData
	{
		GLFWwindow* window;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct VulkanBuffer
	{
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
		void* mappedMemory;
	};

	struct VulkanImage
	{
		VkImage image;
		VkDeviceMemory imageMemory;
		VkFormat imageFormat;
		VkImageView imageView;
	};

	struct VulkanTextureImage
	{
		VulkanImage* image;
		VkSampler textureSampler;
	};

	struct VulkanDescriptorLayoutSettings
	{
		std::vector<uint32_t> binding;
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stageFlags;
		std::vector<uint32_t> descriptorCount;
	};

	struct VulkanPipelineSettings
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineMultisampleStateCreateInfo multisampling;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlending;
		VkPipelineDepthStencilStateCreateInfo depthStencil;

		VulkanDescriptorLayoutSettings descriptorLayoutSettings;
	};

	struct VulkanPipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout vkPipelineLayout;
		VkDescriptorPool vkDescriptorPool;
		VkDescriptorSetLayout vkDescriptorSetLayout;
		std::vector<VkDescriptorSet> vkDescriptorSets{};
		std::vector<VulkanBuffer*> uniformBuffers;
	};

	enum VulkanTopologySelection
	{
		VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST,
		VULKAN_RENDERING_ENGINE_TOPOLOGY_LINE_LIST
	};

	struct RenderingVertex {
		glm::vec3 pos{};
		glm::vec3 color{};
		glm::vec2 texcoord{};
		glm::vec3 normal{};

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};

			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(RenderingVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
			
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(RenderingVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(RenderingVertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(RenderingVertex, texcoord);
			
			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(RenderingVertex, normal);

			return attributeDescriptions;
		}
	};

	class VulkanRenderingEngine
	{
	private:
		const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		GLFWwindow* window = nullptr;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle{};

		static std::vector<char> readShaderFile(std::string path);

		uint32_t currentFrame = 0;
		bool framebufferResized = false;

		// Vulkan instance
		VkInstance vkInstance = VK_NULL_HANDLE;
		
		// Queues
		QueueFamilyIndices graphicsQueueIndices{};
		VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
		VkQueue vkPresentQueue = VK_NULL_HANDLE;

		// Devices
		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
		VkDevice vkLogicalDevice = VK_NULL_HANDLE;

		// Surfaces
		VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

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

		// External callback for rendering
		std::function<void(VkCommandBuffer, uint32_t)> external_callback;

		// Required extensions to be supported
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
		};
		// Required validation layers to be supported
		const std::vector<const char*> vkValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		// Validation layers
		VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;
#ifndef _DEBUG
		const bool enableVkValidationLayers = false;
#else
		const bool enableVkValidationLayers = true;
#endif

		// Leak check counters
		size_t leakBufferCounter = 0;
		size_t leakUniformBufferCounter = 0;
		size_t leakImageCounter = 0;
		size_t leakTextureImageCounter = 0;
		size_t leakPipelineCounter = 0;

		// Physical device functions
		int checkVulkanPhysicalDeviceScore(VkPhysicalDevice device);
		QueueFamilyIndices findVulkanQueueFamilies(VkPhysicalDevice device);
		bool checkVulkanDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails queryVulkanSwapChainSupport(VkPhysicalDevice device);
		VkSampleCountFlagBits getMaxUsableSampleCount();

		// VkFormat functions
		VkFormat findVulkanSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findVulkanSupportedDepthFormat();
		bool doesVulkanFormatHaveStencilComponent(VkFormat format);

		// Swap chain functions
		VkSurfaceFormatKHR chooseVulkanSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseVulkanSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createVulkanSwapChainViews();
		void recreateVulkanSwapChain();
		void cleanupVulkanSwapChain();

		// Command buffer functions
		void recordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		// Shaders functions
		VkShaderModule createVulkanShaderModule(const std::vector<char>& code);

		// Image view functions
		VkImageView createVulkanImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		// Init functions
		bool checkVulkanValidationLayers();
		void initVulkanInstance();
		void initVulkanDevice();
		void createVulkanLogicalDevice();
		void createVulkanSurface();
		void createVulkanSwapChain();
		void createVulkanDefaultGraphicsPipeline();
		void createVulkanRenderPass();
		void createVulkanFramebuffers();
		void createVulkanCommandPools();
		void createVulkanCommandBuffers();
		void createVulkanSyncObjects();
		void createVulkanDepthResources();
		void createMultisampledColorResources();

	public:
		VulkanRenderingEngine() {}

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW();
		
		// Cleanup functions
		void cleanup();
		void cleanupVulkanBuffer(VulkanBuffer* buffer);
		void cleanupVulkanTextureImage(VulkanTextureImage* buffer);
		void cleanupVulkanPipeline(VulkanPipeline* pipeline);
		void cleanupVulkanImage(VulkanImage* image);
		
		// Init
		void init(unsigned int xsize, unsigned int ysize, std::string title);

		// Engine functions
		void drawFrame();
		void SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t)> callback);
		
		// Buffer functions
		VulkanBuffer* createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void bufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size);
		VulkanBuffer* createVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices);
		VulkanBuffer* createVulkanStagingBufferPreMapped(VkDeviceSize dataSize);
		VulkanBuffer* createVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize);

		// Image functions
		VulkanImage* createVulkanImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		VulkanTextureImage* createVulkanTextureImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		void copyVulcanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void appendVulkanImageViewToVulkanTextureImage(VulkanTextureImage* teximage);
		void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);
		void transitionVulkanImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		// Command buffer functions
		VkCommandBuffer beginVulkanSingleTimeCommandsCommandBuffer();
		void endVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer);

		// Pipeline functions
		VulkanPipelineSettings getVulkanDefaultPipelineSettings();
		VulkanPipeline* createVulkanPipelineFromPrealloc(VulkanPipeline* pipeline, VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path);
		VulkanPipeline* createVulkanPipeline(VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path);

		// Pipeline descriptor functions
		void createVulkanDescriptorSetLayout(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);
		void createVulkanUniformBuffers(VulkanPipeline* pipeline);
		void createVulkanDescriptorPool(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);
		void createVulkanDescriptorSets(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);

		// Getters
		VkDevice GetLogicalDevice();
		GLFWwindowData const GetWindow();
		const uint32_t GetMaxFramesInFlight();

		// Utility functions
		uint32_t findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
}