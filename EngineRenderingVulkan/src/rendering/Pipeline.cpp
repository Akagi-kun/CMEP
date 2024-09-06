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
#include <string>
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
		: InstanceOwned(with_instance), HoldsVMA(with_instance->GetGraphicMemoryAllocator())
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		assert(!settings.shader.empty() && "A valid shader for this pipeline is required!");

		/************************************/
		// Create Shaders

		// Vertex stage
		auto vert_shader_module =
			ShaderModule(logical_device, shader_path, std::string(settings.shader) + +"_vert.spv");
		vk::PipelineShaderStageCreateInfo vert_shader_stage_info(
			{},
			vk::ShaderStageFlagBits::eVertex,
			*vert_shader_module.GetHandle(),
			"main"
		);

		// Fragment stage
		auto frag_shader_module =
			ShaderModule(logical_device, shader_path, std::string(settings.shader) + "_frag.spv");
		vk::PipelineShaderStageCreateInfo frag_shader_stage_info(
			{},
			vk::ShaderStageFlagBits::eFragment,
			*frag_shader_module.GetHandle(),
			"main"
		);

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

		vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states);

		/************************************/
		// Create Vertex Input Info

		auto binding_description	= RenderingVertex::GetBindingDescription();
		auto attribute_descriptions = RenderingVertex::GetAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertex_input_info(
			{},
			binding_description,
			attribute_descriptions
		);

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
			vk::DescriptorSetLayoutBinding new_binding(
				binding_idx,
				setting.type,
				setting.descriptor_count,
				setting.stage_flags,
				{}
			);

			bindings.push_back(new_binding);

			vk::DescriptorBindingFlags new_flags = vk::DescriptorBindingFlagBits::ePartiallyBound;
			binding_flags.push_back(new_flags);
		}

		vk::DescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info(binding_flags);

		vk::DescriptorSetLayoutCreateInfo layout_create_info({}, bindings, &layout_flags_info);

		descriptor_set_layout = logical_device->createDescriptorSetLayout(layout_create_info);

		/************************************/
		// Create Graphics Pipeline Layout

		vk::PipelineLayoutCreateInfo pipeline_layout_info({}, *descriptor_set_layout, {});

		pipeline_layout = logical_device->createPipelineLayout(pipeline_layout_info);

		/************************************/
		// Create Graphics Pipeline

		// Make local copy of viewport settings
		const auto viewport_settings = PipelineSettings::GetViewportSettings(settings.extent);

		vk::PipelineViewportStateCreateInfo
			viewport_state({}, 1, &viewport_settings, 1, &settings.scissor, {});

		// Make local copy of input assembly
		const auto input_assembly = PipelineSettings::GetInputAssemblySettings(
			settings.input_topology
		);

		vk::GraphicsPipelineCreateInfo pipeline_info(
			{},
			shader_stages,
			&vertex_input_info,
			&input_assembly,
			{},
			&viewport_state,
			PipelineSettings::GetRasterizerSettings(),
			PipelineSettings::GetMultisamplingSettings(
				instance->GetPhysicalDevice()->GetMSAASamples()
			),
			PipelineSettings::GetDepthStencilSettings(),
			PipelineSettings::GetColorBlendSettings(),
			&dynamic_state,
			*pipeline_layout,
			*with_render_pass->native_handle,
			0,
			{},
			{}
		);

		native_handle = logical_device->createGraphicsPipeline(nullptr, pipeline_info);

		/************************************/
		// Create Descriptor Pool Sizes

		pool_sizes.resize(settings.descriptor_settings.size());
		for (const auto& [binding, setting] : settings.descriptor_settings)
		{
			pool_sizes[binding] = vk::DescriptorPoolSize(
				setting.type,
				setting.descriptor_count * max_frames_in_flight
			);
		}

		/************************************/
	}

	void Pipeline::UpdateDescriptorSets(
		const vk::raii::Device&					logical_device,
		UserData&								from,
		per_frame_array<vk::WriteDescriptorSet> with_writes
	)
	{
		for (uint32_t frame = 0; frame < with_writes.size(); frame++)
		{
			with_writes[frame].dstSet = *from.GetDescriptorSet(frame);
		}

		logical_device.updateDescriptorSets(with_writes, {});
	}

	void Pipeline::BindPipeline(
		UserData&		  from,
		vk::CommandBuffer with_command_buffer,
		uint32_t		  current_frame
	)
	{
		with_command_buffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			*pipeline_layout,
			0,
			*from.GetDescriptorSet(current_frame),
			{}
		);

		with_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *native_handle);
	}

	Pipeline::UserData* Pipeline::AllocateNewUserData()
	{
		auto* into = new UserData();

		// Allocate uniform buffers
		static constexpr vk::DeviceSize buffer_size = sizeof(RendererMatrixData);
		for (size_t i = 0; i < max_frames_in_flight; i++)
		{
			into->uniform_buffers[i] = new UniformBuffer(instance, buffer_size);
		}

		AllocateNewDescriptorPool(*into);
		AllocateNewDescriptorSets(*into);

		// Set up binding 0 to point to uniform buffers
		per_frame_array<vk::DescriptorBufferInfo> descriptor_buffer_infos{};
		per_frame_array<vk::WriteDescriptorSet>	  descriptor_writes{};

		for (uint32_t frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx++)
		{
			descriptor_buffer_infos[frame_idx] = vk::DescriptorBufferInfo(
				*into->uniform_buffers[frame_idx]->GetHandle(),
				0,
				sizeof(RendererMatrixData)
			);

			descriptor_writes[frame_idx] = vk::WriteDescriptorSet(
				{},
				0,
				0,
				vk::DescriptorType::eUniformBuffer,
				{},
				descriptor_buffer_infos[frame_idx],
				{}
			);
		}

		UpdateDescriptorSets(*instance->GetLogicalDevice(), *into, descriptor_writes);

		return into;
	}

#pragma endregion

#pragma region Private

	void Pipeline::AllocateNewDescriptorPool(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::DescriptorPoolCreateInfo pool_create_info(
			{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet},
			max_frames_in_flight,
			pool_sizes
		);

		data_ref.descriptor_pool = logical_device->createDescriptorPool(pool_create_info);
	}

	void Pipeline::AllocateNewDescriptorSets(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		std::vector<vk::DescriptorSetLayout> layouts(max_frames_in_flight, *descriptor_set_layout);
		vk::DescriptorSetAllocateInfo		 alloc_info(*data_ref.descriptor_pool, layouts);

		data_ref.descriptor_sets = vk::raii::DescriptorSets(*logical_device, alloc_info);
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
