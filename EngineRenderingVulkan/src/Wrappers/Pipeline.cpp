#include "Wrappers/Pipeline.hpp"

#include "ImportVulkan.hpp"
#include "VulkanStructDefs.hpp"
#include "Wrappers/Buffer.hpp"
#include "Wrappers/HoldsVMA.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/LogicalDevice.hpp"
#include "Wrappers/RenderPass.hpp"
#include "Wrappers/ShaderModule.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Pipeline::Pipeline(
		InstanceOwned::value_t with_instance,
		RenderPass* with_render_pass,
		VulkanPipelineSettings settings,
		const std::filesystem::path& shader_path
	)
		: InstanceOwned(with_instance), HoldsVMA(with_instance->GetGraphicMemoryAllocator())
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		assert(!settings.shader.empty() && "A valid shader for this pipeline is required!");

		/************************************/
		// Create Shaders

		// Vertex stage
		auto vert_shader_module = ShaderModule(instance, shader_path, std::string(settings.shader) + +"_vert.spv");
		vk::PipelineShaderStageCreateInfo
			vert_shader_stage_info({}, vk::ShaderStageFlagBits::eVertex, vert_shader_module.GetHandle(), "main");

		// Fragment stage
		auto frag_shader_module = ShaderModule(instance, shader_path, std::string(settings.shader) + "_frag.spv");
		vk::PipelineShaderStageCreateInfo
			frag_shader_stage_info({}, vk::ShaderStageFlagBits::eFragment, frag_shader_module.GetHandle(), "main");

		std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages = {
			vert_shader_stage_info,
			frag_shader_stage_info
		};

		/************************************/
		// Create Dynamic States

		std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

		vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states);

		/************************************/
		// Create Vertex Input Info

		auto binding_description	= RenderingVertex::GetBindingDescription();
		auto attribute_descriptions = RenderingVertex::GetAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, binding_description, attribute_descriptions);

		/************************************/
		// Create Vulkan Descriptor Set Layout

		// Binding 0 of vertex shader always must be uniform buffer
		settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}
		);

		std::vector<vk::DescriptorSetLayoutBinding> bindings  = {};
		std::vector<vk::DescriptorBindingFlags> binding_flags = {};

		for (auto& setting : settings.descriptor_layout_settings)
		{
			vk::DescriptorSetLayoutBinding
				new_binding(setting.binding, setting.type, setting.descriptor_count, setting.stage_flags, {});

			bindings.push_back(new_binding);

			vk::DescriptorBindingFlags new_flags = vk::DescriptorBindingFlagBits::ePartiallyBound;
			binding_flags.push_back(new_flags);
		}

		vk::DescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info(binding_flags);

		vk::DescriptorSetLayoutCreateInfo layout_create_info({}, bindings, &layout_flags_info);

		descriptor_set_layout = logical_device->GetHandle().createDescriptorSetLayout(layout_create_info);

		/************************************/
		// Create Graphics Pipeline Layout

		vk::PipelineLayoutCreateInfo pipeline_layout_info({}, descriptor_set_layout, {});
		pipeline_layout = logical_device->GetHandle().createPipelineLayout(pipeline_layout_info);

		/************************************/
		// Create Graphics Pipeline

		vk::PipelineViewportStateCreateInfo viewport_state(
			{},
			1,
			VulkanPipelineSettings::GetViewportSettings(settings.extent),
			1,
			&settings.scissor,
			{}
		);

		vk::GraphicsPipelineCreateInfo pipeline_info(
			{},
			shader_stages,
			&vertex_input_info,
			VulkanPipelineSettings::GetInputAssemblySettings(settings.input_topology),
			{},
			&viewport_state,
			VulkanPipelineSettings::GetRasterizerSettings(),
			VulkanPipelineSettings::GetMultisamplingSettings(instance->GetMSAASamples()),
			VulkanPipelineSettings::GetDepthStencilSettings(),
			VulkanPipelineSettings::GetColorBlendSettings(),
			&dynamic_state,
			pipeline_layout,
			with_render_pass->native_handle,
			0,
			{},
			{}
		);

		vk::Result result;
		std::tie(result, native_handle) = logical_device->GetHandle().createGraphicsPipeline(nullptr, pipeline_info);
		if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed creating pipeline!");
		}

		/************************************/
		// Create Descriptor Pool Sizes

		pool_sizes.resize(settings.descriptor_layout_settings.size());
		for (size_t i = 0; i < settings.descriptor_layout_settings.size(); i++)
		{
			pool_sizes[i] = vk::DescriptorPoolSize(
				settings.descriptor_layout_settings[i].type,
				settings.descriptor_layout_settings[i].descriptor_count * max_frames_in_flight
			);
		}

		/************************************/
	}

	Pipeline::~Pipeline()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().waitIdle();

		logical_device->GetHandle().destroyDescriptorSetLayout(descriptor_set_layout);

		logical_device->GetHandle().destroyPipeline(native_handle);
		logical_device->GetHandle().destroyPipelineLayout(pipeline_layout);
	}

	void Pipeline::UpdateDescriptorSets(
		vk::Device logical_device,
		UserData& from,
		per_frame_array<vk::WriteDescriptorSet> with_writes
	)
	{
		for (uint32_t frame = 0; frame < with_writes.size(); frame++)
		{
			with_writes[frame].dstSet = from.GetDescriptorSet(frame);
		}

		logical_device.updateDescriptorSets(with_writes, {});
	}

	void Pipeline::BindPipeline(UserData& from, vk::CommandBuffer with_command_buffer, uint32_t current_frame)
	{
		with_command_buffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			pipeline_layout,
			0,
			from.descriptor_sets[current_frame],
			{}
		);

		with_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, native_handle);
	}

	void Pipeline::AllocateNewUserData(UserData& into)
	{
		auto& allocated_user_data = into;

		this->AllocateNewUniformBuffers(allocated_user_data.uniform_buffers);
		this->AllocateNewDescriptorPool(allocated_user_data);
		this->AllocateNewDescriptorSets(allocated_user_data);

		// Set up binding 0 to point to uniform buffers
		per_frame_array<vk::DescriptorBufferInfo> descriptor_buffer_infos{};
		per_frame_array<vk::WriteDescriptorSet> descriptor_writes{};

		for (uint32_t frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx++)
		{
			descriptor_buffer_infos[frame_idx] = vk::DescriptorBufferInfo(
				allocated_user_data.uniform_buffers[frame_idx]->GetHandle(),
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

		UpdateDescriptorSets(instance->GetLogicalDevice()->GetHandle(), allocated_user_data, descriptor_writes);
	}

#pragma endregion

#pragma region Private

	void Pipeline::AllocateNewUniformBuffers(per_frame_array<Buffer*>& buffer_ref)
	{
		static constexpr VkDeviceSize buffer_size = sizeof(RendererMatrixData);

		for (size_t i = 0; i < max_frames_in_flight; i++)
		{
			// TODO: Create UniformBuffer class?
			buffer_ref[i] = new Buffer(
				instance,
				buffer_size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);
		}
	}

	void Pipeline::AllocateNewDescriptorPool(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::DescriptorPoolCreateInfo pool_create_info({}, max_frames_in_flight, pool_sizes);

		data_ref.descriptor_pool = logical_device->GetHandle().createDescriptorPool(pool_create_info);
	}

	void Pipeline::AllocateNewDescriptorSets(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		std::vector<vk::DescriptorSetLayout> layouts(max_frames_in_flight, this->descriptor_set_layout);
		vk::DescriptorSetAllocateInfo alloc_info(data_ref.descriptor_pool, layouts);

		auto allocated_sets = logical_device->GetHandle().allocateDescriptorSets(alloc_info);

		// move allocated descriptor sets into the user data
		std::copy_n(
			std::make_move_iterator(allocated_sets.begin()),
			data_ref.descriptor_sets.size(),
			data_ref.descriptor_sets.begin()
		);
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
