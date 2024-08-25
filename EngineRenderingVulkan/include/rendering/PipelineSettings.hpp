#pragma once

#include "vulkan/vulkan.hpp"

#include <map>
#include <optional>

namespace Engine::Rendering::Vulkan
{
	struct DescriptorBindingSetting
	{
		uint32_t descriptor_count;
		vk::DescriptorType type;
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

			return /* (binding == other.binding) && */ (descriptor_count == other.descriptor_count) &&
				   (type == other.type) && (stage_flags == other.stage_flags) && opt_match;
		}
	};

	struct PipelineSettings
	{
		vk::PrimitiveTopology input_topology;
		vk::Extent2D extent;
		std::string shader;
		vk::Rect2D scissor;
		// maps binding->setting
		std::map<uint32_t, DescriptorBindingSetting> descriptor_settings;

		PipelineSettings() = default;
		PipelineSettings(
			const vk::Extent2D with_extent,
			const std::string_view with_shader,
			const vk::PrimitiveTopology with_topology
		)
			: input_topology(with_topology), extent(with_extent), shader(with_shader),
			  scissor(vk::Rect2D{{0, 0}, with_extent})
		{
		}

		bool operator==(const PipelineSettings& other) const
		{
			bool topo_match = (input_topology == other.input_topology);

			bool extent_match = ((extent.width == other.extent.width) && (extent.height == other.extent.height));

			bool shader_match = (shader == other.shader);

			bool scissor_match = (scissor.extent.width == other.scissor.extent.width) &&
								 (scissor.extent.height == other.scissor.extent.height) &&
								 (scissor.offset.x == other.scissor.offset.x) &&
								 (scissor.offset.y == other.scissor.offset.y);

			// If this results in false, the final value will also have to be false
			bool settings_match = descriptor_settings.size() == other.descriptor_settings.size();

			// O(pow(N, 2))
			for (const auto& [binding, setting] : descriptor_settings)
			{
				// Check every value of other for a match of value in this
				bool tmp_bool = false;
				for (const auto& [other_binding, other_setting] : other.descriptor_settings)
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

			return topo_match && extent_match && shader_match && scissor_match && settings_match;
		}

		static vk::PipelineInputAssemblyStateCreateInfo GetInputAssemblySettings(
			vk::PrimitiveTopology with_topology,
			bool enable_primitive_restart = false
		)
		{
			vk::PipelineInputAssemblyStateCreateInfo input_assembly(
				{},
				with_topology,
				static_cast<vk::Bool32>(enable_primitive_restart)
			);

			return input_assembly;
		}

		static vk::Viewport GetViewportSettings(vk::Extent2D extent)
		{
			vk::Viewport
				viewport(0.f, 0.f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.f, 1.f);

			return viewport;
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
				vk::False,
				{},
				{},
				{},
				1.f,
				{}
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
				PipelineSettings::GetColorBlendAttachmentSettings(),
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
} // namespace Engine::Rendering::Vulkan
