#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "vulkan/vulkan.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace Engine::Rendering::Vulkan
{
	struct DescriptorBindingSetting
	{
		uint32_t			 descriptor_count;
		vk::DescriptorType	 type;
		vk::ShaderStageFlags stage_flags;

		// Specify this only if this binding has some identifiable information
		// that should be used in operator== to compare with other settings
		// if .has_value() is false, this hash will be ignored
		std::optional<size_t> opt_match_hash;

		bool operator==(const DescriptorBindingSetting& other) const
		{
			bool opt_match = true;
			if (opt_match_hash.has_value() && other.opt_match_hash.has_value())
			{
				opt_match = (opt_match_hash.value() == other.opt_match_hash.value());
			}

			return (descriptor_count == other.descriptor_count) && (type == other.type) &&
				   (stage_flags == other.stage_flags) && opt_match;
		}
	};

	struct PipelineSettings
	{
		vk::PrimitiveTopology input_topology;
		vk::Extent2D		  extent;
		std::string			  shader;
		vk::Rect2D			  scissor;
		// maps binding->setting
		std::map<uint32_t, DescriptorBindingSetting> descriptor_settings;

		PipelineSettings() = default;
		PipelineSettings(
			const vk::Extent2D			with_extent,
			const std::string_view		with_shader,
			const vk::PrimitiveTopology with_topology
		)
			: input_topology(with_topology), extent(with_extent), shader(with_shader),
			  scissor(vk::Rect2D{{0, 0}, with_extent})
		{}

		bool operator==(const PipelineSettings& other) const
		{
			bool topo_match = (input_topology == other.input_topology);

			bool extent_match =
				((extent.width == other.extent.width) &&
				 (extent.height == other.extent.height));

			bool shader_match = (shader == other.shader);

			bool scissor_match = (scissor.extent.width == other.scissor.extent.width) &&
								 (scissor.extent.height == other.scissor.extent.height) &&
								 (scissor.offset.x == other.scissor.offset.x) &&
								 (scissor.offset.y == other.scissor.offset.y);

			// If this results in false, the final value will also have to be false
			bool settings_match = descriptor_settings.size() ==
								  other.descriptor_settings.size();

			// O(pow(N, 2))
			for (const auto& [binding, setting] : descriptor_settings)
			{
				// Check every value of other for a match of value in this
				bool tmp_bool = false;
				for (const auto& [other_binding, other_setting] :
					 other.descriptor_settings)
				{
					if (binding == other_binding && setting == other_setting)
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

			return topo_match && extent_match && shader_match && scissor_match &&
				   settings_match;
		}

		static vk::PipelineInputAssemblyStateCreateInfo getInputAssemblySettings(
			vk::PrimitiveTopology with_topology,
			bool				  enable_primitive_restart = false
		)
		{
			vk::PipelineInputAssemblyStateCreateInfo input_assembly{
				.topology = with_topology,
				.primitiveRestartEnable = static_cast<vk::Bool32>(enable_primitive_restart)
			};

			return input_assembly;
		}

		static vk::Viewport getViewportSettings(vk::Extent2D extent)
		{
			vk::Viewport viewport{
				.x		  = 0,
				.y		  = 0,
				.width	  = static_cast<float>(extent.width),
				.height	  = static_cast<float>(extent.height),
				.minDepth = 0.f,
				.maxDepth = 1.f
			};

			return viewport;
		}

		static const vk::PipelineRasterizationStateCreateInfo* getRasterizerSettings()
		{
			static vk::PipelineRasterizationStateCreateInfo rasterizer{
				.depthClampEnable		 = vk::False,
				.rasterizerDiscardEnable = vk::False,
				.polygonMode			 = vk::PolygonMode::eFill,
				.cullMode				 = vk::CullModeFlagBits::eFront,
				.frontFace				 = vk::FrontFace::eClockwise,
				.depthBiasEnable		 = vk::False,
				.depthBiasConstantFactor = {},
				.depthBiasClamp			 = {},
				.depthBiasSlopeFactor	 = {},
				.lineWidth				 = 1.f
			};

			return &rasterizer;
		}

		static const vk::PipelineMultisampleStateCreateInfo*
		getMultisamplingSettings(vk::SampleCountFlagBits msaa_samples)
		{
			static vk::PipelineMultisampleStateCreateInfo multisampling{
				.rasterizationSamples  = msaa_samples,
				.sampleShadingEnable   = vk::False,
				.minSampleShading	   = 1.f,
				.pSampleMask		   = {},
				.alphaToCoverageEnable = {},
				.alphaToOneEnable	   = {}
			};

			return &multisampling;
		}

		static const vk::PipelineColorBlendAttachmentState*
		getColorBlendAttachmentSettings()
		{
			static vk::PipelineColorBlendAttachmentState color_blend_attachment{
				.blendEnable		 = vk::True,
				.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
				.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.colorBlendOp		 = vk::BlendOp::eAdd,
				.srcAlphaBlendFactor = vk::BlendFactor::eOne,
				.dstAlphaBlendFactor = vk::BlendFactor::eZero,
				.alphaBlendOp		 = vk::BlendOp::eAdd,
				.colorWriteMask =
					vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
					vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
			};

			return &color_blend_attachment;
		}

		static const vk::PipelineColorBlendStateCreateInfo* getColorBlendSettings()
		{
			static vk::PipelineColorBlendStateCreateInfo color_blending{
				.logicOpEnable	 = vk::False,
				.logicOp		 = vk::LogicOp::eClear,
				.attachmentCount = 1,
				.pAttachments	 = PipelineSettings::getColorBlendAttachmentSettings(),
				.blendConstants	 = {{0.f, 0.f, 0.f, 0.f}}
			};

			return &color_blending;
		}

		static const vk::PipelineDepthStencilStateCreateInfo* getDepthStencilSettings()
		{
			static vk::PipelineDepthStencilStateCreateInfo depth_stencil{
				.depthTestEnable	   = vk::True,
				.depthWriteEnable	   = vk::True,
				.depthCompareOp		   = vk::CompareOp::eLess,
				.depthBoundsTestEnable = vk::False,
				.stencilTestEnable	   = vk::False,
				.front				   = {},
				.back				   = {},
				.minDepthBounds		   = 0.f,
				.maxDepthBounds		   = 1.f
			};

			return &depth_stencil;
		}
	};
} // namespace Engine::Rendering::Vulkan
