#include "rendering/Pipeline.hpp"

#include "fwd.hpp"

#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/StructDefs.hpp"
#include "objects/Buffer.hpp"
#include "rendering/PipelineSettings.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/ShaderModule.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Pipeline::Pipeline(
		InstanceOwned::value_t		 with_instance,
		RenderPass*					 with_render_pass,
		PipelineSettings			 settings,
		const std::filesystem::path& shader_path
	)
		: InstanceOwned(with_instance),
		  HoldsVMA(with_instance->getGraphicMemoryAllocator())
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		assert(!settings.shader.empty() && "A valid shader for this pipeline is required!");

		/************************************/
		// Create Shaders

		// Vertex stage
		auto vert_shader_module =
			ShaderModule(logical_device, shader_path / (settings.shader + "_vert.spv"));
		vk::PipelineShaderStageCreateInfo vert_shader_stage_info{
			.stage	= vk::ShaderStageFlagBits::eVertex,
			.module = *vert_shader_module.getHandle(),
			.pName	= "main"
		};

		// Fragment stage
		auto frag_shader_module =
			ShaderModule(logical_device, shader_path / (settings.shader + "_frag.spv"));
		vk::PipelineShaderStageCreateInfo frag_shader_stage_info{
			.stage	= vk::ShaderStageFlagBits::eFragment,
			.module = *frag_shader_module.getHandle(),
			.pName	= "main"
		};

		std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages = {
			vert_shader_stage_info,
			frag_shader_stage_info
		};

		/************************************/
		// Create Dynamic States

		std::vector<vk::DynamicState> dynamic_states = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamic_state{
			.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
			.pDynamicStates	   = dynamic_states.data()
		};

		/************************************/
		// Create Vertex Input Info

		auto binding_description	= RenderingVertex::getBindingDescription();
		auto attribute_descriptions = RenderingVertex::getAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertex_input_info{
			.vertexBindingDescriptionCount =
				static_cast<uint32_t>(binding_description.size()),
			.pVertexBindingDescriptions = binding_description.data(),
			.vertexAttributeDescriptionCount =
				static_cast<uint32_t>(attribute_descriptions.size()),
			.pVertexAttributeDescriptions = attribute_descriptions.data(),
		};

		/************************************/
		// Create Vulkan Descriptor Set Layout

		// Binding 0 of vertex shader always must be uniform buffer
		settings.descriptor_settings.emplace(
			0,
			DescriptorBindingSetting{
				1,
				vk::DescriptorType::eUniformBuffer,
				vk::ShaderStageFlagBits::eVertex,
				{}
			}
		);

		std::vector<vk::DescriptorSetLayoutBinding> bindings	  = {};
		std::vector<vk::DescriptorBindingFlags>		binding_flags = {};

		for (const auto& [binding_idx, setting] : settings.descriptor_settings)
		{
			vk::DescriptorSetLayoutBinding new_binding{
				.binding		 = binding_idx,
				.descriptorType	 = setting.type,
				.descriptorCount = setting.descriptor_count,
				.stageFlags		 = setting.stage_flags,
			};

			bindings.push_back(new_binding);

			vk::DescriptorBindingFlags new_flags =
				vk::DescriptorBindingFlagBits::ePartiallyBound;
			binding_flags.push_back(new_flags);
		}

		vk::DescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info{
			.bindingCount  = static_cast<uint32_t>(binding_flags.size()),
			.pBindingFlags = binding_flags.data()
		};

		vk::DescriptorSetLayoutCreateInfo layout_create_info{
			.pNext		  = &layout_flags_info,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings	  = bindings.data()
		};

		descriptor_set_layout =
			logical_device->createDescriptorSetLayout(layout_create_info);

		/************************************/
		// Create Graphics Pipeline Layout

		vk::PipelineLayoutCreateInfo pipeline_layout_info{
			.setLayoutCount = 1,
			.pSetLayouts	= &*descriptor_set_layout
		};

		pipeline_layout = logical_device->createPipelineLayout(pipeline_layout_info);

		/************************************/
		// Create Graphics Pipeline

		// Make local copy of viewport settings
		const auto viewport_settings =
			PipelineSettings::getViewportSettings(settings.extent);

		vk::PipelineViewportStateCreateInfo viewport_state{
			.viewportCount = 1,
			.pViewports	   = &viewport_settings,
			.scissorCount  = 1,
			.pScissors	   = &settings.scissor
		};

		// Make local copy of input assembly
		const auto input_assembly =
			PipelineSettings::getInputAssemblySettings(settings.input_topology);

		vk::GraphicsPipelineCreateInfo pipeline_info{
			.stageCount			 = static_cast<uint32_t>(shader_stages.size()),
			.pStages			 = shader_stages.data(),
			.pVertexInputState	 = &vertex_input_info,
			.pInputAssemblyState = &input_assembly,
			.pTessellationState	 = {},
			.pViewportState		 = &viewport_state,
			.pRasterizationState = PipelineSettings::getRasterizerSettings(),
			.pMultisampleState	 = PipelineSettings::getMultisamplingSettings(
				  instance->getPhysicalDevice()->getMSAASamples()
			  ),
			.pDepthStencilState = PipelineSettings::getDepthStencilSettings(),
			.pColorBlendState	= PipelineSettings::getColorBlendSettings(),
			.pDynamicState		= &dynamic_state,
			.layout				= *pipeline_layout,
			.renderPass			= *with_render_pass->getHandle(),
			.subpass			= 0,
		};

		native_handle = logical_device->createGraphicsPipeline(nullptr, pipeline_info);

		/************************************/
		// Create Descriptor Pool Sizes

		pool_sizes.resize(settings.descriptor_settings.size());
		for (const auto& [binding, setting] : settings.descriptor_settings)
		{
			pool_sizes[binding] = vk::DescriptorPoolSize{
				.type			 = setting.type,
				.descriptorCount = setting.descriptor_count * max_frames_in_flight
			};
		}

		/************************************/
	}

	void Pipeline::updateDescriptorSets(
		const vk::raii::Device&					logical_device,
		UserData&								from,
		per_frame_array<vk::WriteDescriptorSet> with_writes
	)
	{
		for (uint32_t frame = 0; frame < with_writes.size(); frame++)
		{
			with_writes[frame].dstSet = *from.getDescriptorSet(frame);
		}

		logical_device.updateDescriptorSets(with_writes, {});
	}

	void Pipeline::bindPipeline(
		UserData&		  userdata_ref,
		vk::CommandBuffer with_command_buffer,
		uint32_t		  current_frame
	)
	{
		with_command_buffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			*pipeline_layout,
			0,
			*userdata_ref.getDescriptorSet(current_frame),
			{}
		);

		with_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *native_handle);
	}

	Pipeline::UserData* Pipeline::allocateNewUserData()
	{
		auto* into = new UserData();

		// Allocate uniform buffers
		static constexpr vk::DeviceSize buffer_size = sizeof(RendererMatrixData);
		for (size_t i = 0; i < max_frames_in_flight; i++)
		{
			into->uniform_buffers[i] = new UniformBuffer(
				instance->getLogicalDevice(),
				instance->getGraphicMemoryAllocator(),
				buffer_size
			);
		}

		allocateNewDescriptorPool(*into);
		allocateNewDescriptorSets(*into);

		// Set up binding 0 to point to uniform buffers
		per_frame_array<vk::DescriptorBufferInfo> descriptor_buffer_infos{};
		per_frame_array<vk::WriteDescriptorSet>	  descriptor_writes{};

		for (uint32_t frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx++)
		{
			descriptor_buffer_infos[frame_idx] = vk::DescriptorBufferInfo{
				.buffer = *into->uniform_buffers[frame_idx]->getHandle(),
				.offset = 0,
				.range	= sizeof(RendererMatrixData)
			};

			descriptor_writes[frame_idx] = vk::WriteDescriptorSet{
				.dstSet			 = {},
				.dstBinding		 = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType	 = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo	 = &descriptor_buffer_infos[frame_idx]
			};
		}

		updateDescriptorSets(*instance->getLogicalDevice(), *into, descriptor_writes);

		return into;
	}

#pragma endregion

#pragma region Private

	void Pipeline::allocateNewDescriptorPool(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		vk::DescriptorPoolCreateInfo pool_create_info{
			.flags		   = {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet},
			.maxSets	   = max_frames_in_flight,
			.poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
			.pPoolSizes	   = pool_sizes.data()
		};

		data_ref.descriptor_pool = logical_device->createDescriptorPool(pool_create_info);
	}

	void Pipeline::allocateNewDescriptorSets(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		std::vector<vk::DescriptorSetLayout> layouts(
			max_frames_in_flight,
			*descriptor_set_layout
		);
		vk::DescriptorSetAllocateInfo alloc_info{
			.descriptorPool		= *data_ref.descriptor_pool,
			.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts		= layouts.data()
		};

		data_ref.descriptor_sets = vk::raii::DescriptorSets(*logical_device, alloc_info);
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
