#pragma once

#include "ImportVulkan.hpp"
#include "glm/glm.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace Engine::Rendering
{
	static constexpr uint16_t max_frames_in_flight		 = 3;
	template <typename value_type> using per_frame_array = std::array<value_type, max_frames_in_flight>;

	struct RendererMatrixData
	{
		glm::mat4 mat_vp{};
		glm::mat4 mat_model{};
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

	struct SyncObjects
	{
		VkSemaphore image_available;
		VkSemaphore present_ready; // render_finished_semaphores
		VkFence in_flight;
	};

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;
	};

	struct VulkanDescriptorLayoutSettings
	{
		uint32_t binding;
		uint32_t descriptor_count;
		vk::DescriptorType type;
		vk::ShaderStageFlags stage_flags;

		bool operator==(const VulkanDescriptorLayoutSettings& other) const
		{
			return (binding == other.binding) && (descriptor_count == other.descriptor_count) && (type == other.type) &&
				   (stage_flags == other.stage_flags);
		}
	};

	struct VulkanPipelineSettings
	{
		vk::PrimitiveTopology input_topology;
		VkExtent2D extent;
		std::string shader;
		vk::Rect2D scissor;
		std::vector<VulkanDescriptorLayoutSettings> descriptor_layout_settings;

		VulkanPipelineSettings() = default;
		VulkanPipelineSettings(
			const vk::Extent2D with_extent,
			const std::string_view with_shader,
			const vk::PrimitiveTopology with_topology =
				vk::PrimitiveTopology::eTriangleList // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		)
			: input_topology(with_topology), extent(with_extent), shader(with_shader),
			  scissor(vk::Rect2D{{0, 0}, with_extent})
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

		static const vk::PipelineInputAssemblyStateCreateInfo* GetInputAssemblySettings(
			vk::PrimitiveTopology with_topology,
			bool enable_primitive_restart = false
		)
		{
			static vk::PipelineInputAssemblyStateCreateInfo input_assembly(
				{},
				with_topology,
				static_cast<vk::Bool32>(enable_primitive_restart)
			);

			return &input_assembly;
		}

		static const vk::Viewport* GetViewportSettings(VkExtent2D extent)
		{
			static vk::Viewport
				viewport(0.f, 0.f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.f, 1.f);

			return &viewport;
		}

		static const vk::PipelineRasterizationStateCreateInfo* GetRasterizerSettings()
		{
			static vk::PipelineRasterizationStateCreateInfo rasterizer(
				{},
				vk::False,
				vk::False,
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eFront,
				vk::FrontFace::eClockwise,
				vk::False
			);

			return &rasterizer;
		}

		static const vk::PipelineMultisampleStateCreateInfo* GetMultisamplingSettings(
			vk::SampleCountFlagBits msaa_samples
		)
		{
			static vk::PipelineMultisampleStateCreateInfo multisampling({}, msaa_samples, vk::False, 1.f, {}, {}, {});

			return &multisampling;
		}

		static const vk::PipelineColorBlendAttachmentState* GetColorBlendAttachmentSettings()
		{
			static vk::PipelineColorBlendAttachmentState color_blend_attachment(
				vk::True,
				vk::BlendFactor::eSrcAlpha,
				vk::BlendFactor::eOneMinusSrcAlpha,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
					vk::ColorComponentFlagBits::eA
			);

			return &color_blend_attachment;
		}

		static const vk::PipelineColorBlendStateCreateInfo* GetColorBlendSettings()
		{
			static vk::PipelineColorBlendStateCreateInfo color_blending(
				{},
				vk::False,
				vk::LogicOp::eClear,
				1,
				VulkanPipelineSettings::GetColorBlendAttachmentSettings(),
				{0.f}
			);

			return &color_blending;
		}

		static const vk::PipelineDepthStencilStateCreateInfo* GetDepthStencilSettings()
		{
			static vk::PipelineDepthStencilStateCreateInfo
				depth_stencil({}, vk::True, vk::True, vk::CompareOp::eLess, vk::False, vk::False, {}, {}, 0.f, 1.f, {});

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

		static constexpr std::array<vk::VertexInputBindingDescription, 1> GetBindingDescription()
		{
			return {
				vk::VertexInputBindingDescription{
					0,
					sizeof(RenderingVertex),
					vk::VertexInputRate::eVertex,
				},
			};
		}

		static constexpr std::array<vk::VertexInputAttributeDescription, 4> GetAttributeDescriptions()
		{
			return {
				vk::VertexInputAttributeDescription{
					0,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(RenderingVertex, pos),
				},
				vk::VertexInputAttributeDescription{
					1,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(RenderingVertex, color),
				},
				vk::VertexInputAttributeDescription{
					2,
					0,
					vk::Format::eR32G32Sfloat,
					offsetof(RenderingVertex, texcoord),
				},
				vk::VertexInputAttributeDescription{
					3,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(RenderingVertex, normal),
				}
			};
		}
	};
} // namespace Engine::Rendering
