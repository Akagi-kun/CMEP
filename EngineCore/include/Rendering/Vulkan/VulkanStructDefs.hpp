#pragma once

#include <optional>
#include <vector>
#include <array>

#include "glm/glm.hpp"

#include "ImportVulkan.hpp"

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
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		void* mappedData;
	};

	struct VulkanImage
	{
		VkImage image;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		VkFormat imageFormat;
		VkImageView imageView;
	};

	struct VulkanTextureImage
	{
		VulkanImage* image;
		VkSampler textureSampler;
		VkFilter useFilter;
		VkSamplerAddressMode useAddressMode;
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
}