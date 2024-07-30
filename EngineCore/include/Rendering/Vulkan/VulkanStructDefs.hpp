#pragma once

#include "Rendering/Transform.hpp"

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

		[[nodiscard]] bool IsComplete() const
		{
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	struct GLFWwindowData
	{
		GLFWwindow* native_handle;
		ScreenSize size;
		std::string title;
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	struct VulkanDescriptorLayoutSettings
	{
		uint32_t binding;
		uint32_t descriptor_count;
		VkDescriptorType types;
		VkShaderStageFlags stage_flags;
	};

	struct VulkanPipelineSettings
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineMultisampleStateCreateInfo multisampling;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blending;
		VkPipelineDepthStencilStateCreateInfo depth_stencil;

		std::string shader;

		std::vector<VulkanDescriptorLayoutSettings> descriptor_layout_settings;
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

			binding_description.binding	  = 0;
			binding_description.stride	  = sizeof(RenderingVertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

			attribute_descriptions[0].binding  = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[0].offset   = offsetof(RenderingVertex, pos);

			attribute_descriptions[1].binding  = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset   = offsetof(RenderingVertex, color);

			attribute_descriptions[2].binding  = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset   = offsetof(RenderingVertex, texcoord);

			attribute_descriptions[3].binding  = 0;
			attribute_descriptions[3].location = 3;
			attribute_descriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[3].offset   = offsetof(RenderingVertex, normal);

			return attribute_descriptions;
		}
	};
} // namespace Engine::Rendering
