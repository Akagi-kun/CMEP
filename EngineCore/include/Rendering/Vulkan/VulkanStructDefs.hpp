#pragma once

#include "ImportVulkan.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace Engine::Rendering
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool IsComplete()
		{
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	struct GLFWwindowData
	{
		GLFWwindow* window;
		unsigned int window_x = 0, window_y = 0;
		std::string window_title;
	};

	struct SwapChainSupportDetails
	{
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
		VkPipelineLayout vk_pipeline_layout;
		VkDescriptorPool vk_descriptor_pool;
		VkDescriptorSetLayout vk_descriptor_set_layout;
		std::vector<VkDescriptorSet> vk_descriptor_sets;
		std::vector<VulkanBuffer*> uniform_buffers;
	};

	struct RenderingVertex
	{
		glm::vec3 pos{};
		glm::vec3 color{};
		glm::vec2 texcoord{};
		glm::vec3 normal{};

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription binding_description{};

			binding_description.binding = 0;
			binding_description.stride = sizeof(RenderingVertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[0].offset = offsetof(RenderingVertex, pos);

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(RenderingVertex, color);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(RenderingVertex, texcoord);

			attribute_descriptions[3].binding = 0;
			attribute_descriptions[3].location = 3;
			attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[3].offset = offsetof(RenderingVertex, normal);

			return attribute_descriptions;
		}
	};
} // namespace Engine::Rendering
