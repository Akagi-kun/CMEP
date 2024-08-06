#pragma once

#include "Rendering/Transform.hpp"

#include "ImportVulkan.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace Engine::Rendering
{
	struct GLFWwindowData
	{
		GLFWwindow* native_handle;
		ScreenSize size;
		std::string title;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		[[nodiscard]] bool IsComplete() const
		{
			return graphics_family.has_value() && present_family.has_value();
		}
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
		VkDescriptorType type;
		VkShaderStageFlags stage_flags;

		bool operator==(const VulkanDescriptorLayoutSettings& other) const
		{
			return (binding == other.binding) && (descriptor_count == other.descriptor_count) && (type == other.type) &&
				   (stage_flags == other.stage_flags);
		}
	};

	struct VulkanPipelineSettings
	{
		VkPrimitiveTopology input_topology;
		VkExtent2D extent;
		std::string shader;
		VkRect2D scissor;
		std::vector<VulkanDescriptorLayoutSettings> descriptor_layout_settings;

		VulkanPipelineSettings() = default;
		VulkanPipelineSettings(
			const VkExtent2D with_extent,
			const std::string_view with_shader,
			const VkPrimitiveTopology with_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		)
			: input_topology(with_topology), extent(with_extent), shader(with_shader),
			  scissor(VkRect2D{{0, 0}, with_extent})
		{
		}

		bool operator==(const VulkanPipelineSettings& other) const
		{
			bool topo_match = (input_topology == other.input_topology);

			bool extent_match = ((extent.width == other.extent.width) && (extent.height == other.extent.height));

			bool shader_match = (shader == other.shader);

			bool scissor_match = (scissor.extent.width == other.scissor.extent.width) &&
								 (scissor.extent.height == other.scissor.extent.height) &&
								 (scissor.offset.x == other.scissor.offset.x) &&
								 (scissor.offset.y == other.scissor.offset.y);

			// If this results in false, the final value will also have to be false
			bool settings_match = descriptor_layout_settings.size() == other.descriptor_layout_settings.size();

			// O(pow(N, 2))
			for (const auto& setting : descriptor_layout_settings)
			{
				// Check every value of other for a match of value in this
				bool tmp_bool = false;
				for (const auto& other_setting : other.descriptor_layout_settings)
				{
					if (setting == other_setting)
					{
						tmp_bool = true;
						break;
					}
				}

				// If a match was not found, result has to be false
				if (!tmp_bool)
				{
					settings_match = false;
					break;
				}
			}

			return topo_match && extent_match && shader_match && scissor_match && settings_match;
		}

		static VkPipelineInputAssemblyStateCreateInfo* GetInputAssemblySettings(
			VkPrimitiveTopology with_topology,
			bool enable_primitive_restart = false
		)
		{
			static VkPipelineInputAssemblyStateCreateInfo input_assembly{};

			input_assembly.sType				  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			input_assembly.topology				  = with_topology;
			input_assembly.primitiveRestartEnable = static_cast<VkBool32>(enable_primitive_restart);

			return &input_assembly;
		}

		static VkViewport* GetViewportSettings(VkExtent2D extent)
		{
			static VkViewport viewport{};

			viewport.x		  = 0.0f;
			viewport.y		  = 0.0f;
			viewport.width	  = static_cast<float>(extent.width);
			viewport.height	  = static_cast<float>(extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			return &viewport;
		}

		static VkPipelineRasterizationStateCreateInfo* GetRasterizerSettings()
		{
			static VkPipelineRasterizationStateCreateInfo rasterizer{};

			rasterizer.sType				   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable		   = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode			   = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth			   = 1.0f;
			rasterizer.cullMode				   = VK_CULL_MODE_FRONT_BIT;
			rasterizer.frontFace			   = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable		   = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp		   = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor	   = 0.0f; // Optional

			return &rasterizer;
		}

		static VkPipelineMultisampleStateCreateInfo* GetMultisamplingSettings(VkSampleCountFlagBits msaa_samples)
		{
			static VkPipelineMultisampleStateCreateInfo multisampling{};

			multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable	= VK_FALSE;
			multisampling.rasterizationSamples	= msaa_samples;
			multisampling.minSampleShading		= 1.0f;		// Optional
			multisampling.pSampleMask			= nullptr;	// Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable		= VK_FALSE; // Optional

			return &multisampling;
		}

		static VkPipelineColorBlendAttachmentState* GetColorBlendAttachmentSettings()
		{
			static VkPipelineColorBlendAttachmentState color_blend_attachment{};

			color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
													VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			color_blend_attachment.blendEnable		   = VK_TRUE;
			color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			color_blend_attachment.colorBlendOp		   = VK_BLEND_OP_ADD;
			color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			color_blend_attachment.alphaBlendOp		   = VK_BLEND_OP_ADD;

			return &color_blend_attachment;
		}

		static VkPipelineColorBlendStateCreateInfo* GetColorBlendSettings()
		{
			static VkPipelineColorBlendStateCreateInfo color_blending{};
			color_blending.sType			 = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			color_blending.logicOpEnable	 = VK_FALSE;
			color_blending.logicOp			 = VK_LOGIC_OP_COPY;
			color_blending.attachmentCount	 = 1;
			color_blending.pAttachments		 = VulkanPipelineSettings::GetColorBlendAttachmentSettings();
			color_blending.blendConstants[0] = 0.0f; // Optional
			color_blending.blendConstants[1] = 0.0f; // Optional
			color_blending.blendConstants[2] = 0.0f; // Optional
			color_blending.blendConstants[3] = 0.0f; // Optional

			return &color_blending;
		}

		static VkPipelineDepthStencilStateCreateInfo* GetDepthStencilSettings()
		{
			static VkPipelineDepthStencilStateCreateInfo depth_stencil{};

			depth_stencil.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil.depthTestEnable		= VK_TRUE;
			depth_stencil.depthWriteEnable		= VK_TRUE;
			depth_stencil.depthCompareOp		= VK_COMPARE_OP_LESS;
			depth_stencil.depthBoundsTestEnable = VK_FALSE;
			depth_stencil.minDepthBounds		= 0.0f; // Optional
			depth_stencil.maxDepthBounds		= 1.0f; // Optional
			depth_stencil.stencilTestEnable		= VK_FALSE;
			depth_stencil.front					= {}; // Optional
			depth_stencil.back					= {}; // Optional

			return &depth_stencil;
		}
	};

	struct RenderingVertex
	{
		glm::vec3 pos{};
		glm::vec3 color{};
		glm::vec2 texcoord{};
		glm::vec3 normal{};

		RenderingVertex(
			const glm::vec3 with_pos,
			const glm::vec3 with_color	  = {},
			const glm::vec2 with_texcoord = {},
			const glm::vec3 with_normal	  = {}
		)
			: pos(with_pos), color(with_color), texcoord(with_texcoord), normal(with_normal)
		{
		}

		static constexpr VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription binding_description{};

			binding_description.binding	  = 0;
			binding_description.stride	  = sizeof(RenderingVertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		static constexpr std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
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
