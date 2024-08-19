#include "Wrappers/Pipeline.hpp"

// #include "Rendering/Renderers/Renderer.hpp"

#include "ImportVulkan.hpp"
#include "VulkanStructDefs.hpp"
#include "Wrappers/Buffer.hpp"
#include "Wrappers/HoldsVMA.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/LogicalDevice.hpp"
#include "Wrappers/RenderPass.hpp"
#include "Wrappers/ShaderModule.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
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

		// Vertex stage
		auto vert_shader_module = ShaderModule(instance, shader_path, std::string(settings.shader) + +"_vert.spv");

		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module.GetHandle();
		vert_shader_stage_info.pName  = "main";

		// Fragment stage
		auto frag_shader_module = ShaderModule(instance, shader_path, std::string(settings.shader) + "_frag.spv");

		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module.GetHandle();
		frag_shader_stage_info.pName  = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
		std::vector<VkDynamicState> dynamic_states		= {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates	= dynamic_states.data();

		auto binding_description	= RenderingVertex::GetBindingDescription();
		auto attribute_descriptions = RenderingVertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType							  = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount	  = 1;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_info.pVertexBindingDescriptions	  = &binding_description;
		vertex_input_info.pVertexAttributeDescriptions	  = attribute_descriptions.data();

		/************************************/
		// this->CreateVulkanDescriptorSetLayout(pipeline, settings.descriptor_layout_settings);

		// Binding 0 of vertex shader always must be uniform buffer
		settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			0,
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT,
		});

		std::vector<VkDescriptorSetLayoutBinding> bindings	= {};
		std::vector<VkDescriptorBindingFlags> binding_flags = {};

		for (auto& setting : settings.descriptor_layout_settings)
		{
			VkDescriptorSetLayoutBinding new_binding{};
			new_binding.binding			   = setting.binding;
			new_binding.descriptorCount	   = setting.descriptor_count;
			new_binding.descriptorType	   = setting.type;
			new_binding.stageFlags		   = setting.stage_flags;
			new_binding.pImmutableSamplers = nullptr;

			bindings.push_back(new_binding);

			VkDescriptorBindingFlags new_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			binding_flags.push_back(new_flags);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo layout_flags_info{};
		layout_flags_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layout_flags_info.bindingCount	= static_cast<uint32_t>(binding_flags.size());
		layout_flags_info.pBindingFlags = binding_flags.data();

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings	 = bindings.data();
		layout_info.pNext		 = &layout_flags_info;

		if (vkCreateDescriptorSetLayout(
				logical_device->GetHandle(),
				&layout_info,
				nullptr,
				&(this->descriptor_set_layout)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		/************************************/

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount			= 1;
		pipeline_layout_info.pSetLayouts			= &this->descriptor_set_layout;
		pipeline_layout_info.pushConstantRangeCount = 0;	   // Optional
		pipeline_layout_info.pPushConstantRanges	= nullptr; // Optional

		if (vkCreatePipelineLayout(
				logical_device->GetHandle(),
				&pipeline_layout_info,
				nullptr,
				&(this->pipeline_layout)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType		 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports	 = VulkanPipelineSettings::GetViewportSettings(settings.extent);
		viewport_state.scissorCount	 = 1;
		viewport_state.pScissors	 = &settings.scissor;

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType				  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount		  = 2;
		pipeline_info.pStages			  = shader_stages;
		pipeline_info.pVertexInputState	  = &vertex_input_info;
		pipeline_info.pInputAssemblyState = VulkanPipelineSettings::GetInputAssemblySettings(settings.input_topology);
		pipeline_info.pViewportState	  = &viewport_state;
		pipeline_info.pRasterizationState = VulkanPipelineSettings::GetRasterizerSettings();
		pipeline_info.pMultisampleState	 = VulkanPipelineSettings::GetMultisamplingSettings(instance->GetMSAASamples());
		pipeline_info.pDepthStencilState = nullptr; // Optional
		pipeline_info.pColorBlendState	 = VulkanPipelineSettings::GetColorBlendSettings();
		pipeline_info.pDynamicState		 = &dynamic_state;
		pipeline_info.layout			 = this->pipeline_layout;
		pipeline_info.pDepthStencilState = VulkanPipelineSettings::GetDepthStencilSettings();
		pipeline_info.renderPass		 = with_render_pass->native_handle;
		pipeline_info.subpass			 = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipeline_info.basePipelineIndex	 = -1;			   // Optional

		if (vkCreateGraphicsPipelines(
				logical_device->GetHandle(),
				VK_NULL_HANDLE,
				1,
				&pipeline_info,
				nullptr,
				&(this->native_handle)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		/************************************/
		// this->CreateVulkanDescriptorPool(pipeline, settings.descriptor_layout_settings);

		pool_sizes.resize(settings.descriptor_layout_settings.size());
		for (size_t i = 0; i < settings.descriptor_layout_settings.size(); i++)
		{
			VkDescriptorPoolSize pool_size{};
			pool_size.type			  = settings.descriptor_layout_settings[i].type;
			pool_size.descriptorCount = max_frames_in_flight * settings.descriptor_layout_settings[i].descriptor_count;

			pool_sizes[i] = pool_size;
		}

		/************************************/
	}

	Pipeline::~Pipeline()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkDeviceWaitIdle(logical_device->GetHandle());

		for (auto& data_ref : this->user_data)
		{
			for (auto* uniform_buffer : data_ref.uniform_buffers)
			{
				delete uniform_buffer;
			}

			vkDestroyDescriptorPool(logical_device->GetHandle(), data_ref.with_pool, nullptr);
		}

		vkDestroyDescriptorSetLayout(logical_device->GetHandle(), this->descriptor_set_layout, nullptr);

		vkDestroyPipeline(logical_device->GetHandle(), this->native_handle, nullptr);
		vkDestroyPipelineLayout(logical_device->GetHandle(), this->pipeline_layout, nullptr);
	}

	void Pipeline::AllocateNewDescriptorPool(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes	= pool_sizes.data();
		pool_info.maxSets		= max_frames_in_flight;

		if (vkCreateDescriptorPool(logical_device->GetHandle(), &pool_info, nullptr, &(data_ref.with_pool)) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void Pipeline::AllocateNewUniformBuffers(per_frame_array<Buffer*>& buffer_ref)
	{
		static constexpr VkDeviceSize buffer_size = sizeof(RendererMatrixData);

		for (size_t i = 0; i < max_frames_in_flight; i++)
		{
			buffer_ref[i] = new Buffer(
				instance,
				buffer_size,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
		}
	}

	void Pipeline::AllocateNewDescriptorSets(Pipeline::UserData& data_ref)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		std::vector<VkDescriptorSetLayout> layouts(max_frames_in_flight, this->descriptor_set_layout);

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool	  = data_ref.with_pool;
		alloc_info.descriptorSetCount = max_frames_in_flight;
		alloc_info.pSetLayouts		  = layouts.data();

		VkResult create_result =
			vkAllocateDescriptorSets(logical_device->GetHandle(), &alloc_info, data_ref.descriptor_sets.data());

		if (create_result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	size_t Pipeline::AllocateNewUserData()
	{
		// TODO: Support actually removing userdata
		size_t user_index = user_data.size();
		user_data.emplace_back();

		auto& allocated_user_data = user_data[user_index];

		this->AllocateNewUniformBuffers(allocated_user_data.uniform_buffers);
		this->AllocateNewDescriptorPool(allocated_user_data);
		this->AllocateNewDescriptorSets(allocated_user_data);

		// Set up binding 0 to point to uniform buffers
		per_frame_array<VkDescriptorBufferInfo> descriptor_buffer_infos{};
		per_frame_array<VkWriteDescriptorSet> descriptor_writes{};

		for (uint32_t frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx++)
		{
			descriptor_buffer_infos[frame_idx].buffer = *allocated_user_data.uniform_buffers[frame_idx];
			descriptor_buffer_infos[frame_idx].offset = 0;
			descriptor_buffer_infos[frame_idx].range  = sizeof(RendererMatrixData);

			descriptor_writes[frame_idx].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[frame_idx].dstBinding		 = 0;
			descriptor_writes[frame_idx].dstArrayElement = 0;
			descriptor_writes[frame_idx].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[frame_idx].descriptorCount = 1;
			descriptor_writes[frame_idx].pBufferInfo	 = &(descriptor_buffer_infos[frame_idx]);
		}

		this->UpdateDescriptorSets(user_index, descriptor_writes);

		return user_index;
	}

	void Pipeline::UpdateDescriptorSets(size_t user_index, per_frame_array<VkWriteDescriptorSet> with_writes)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		for (uint32_t frame = 0; frame < with_writes.size(); frame++)
		{
			with_writes[frame].dstSet = this->GetDescriptorSet(user_index, frame);
		}

		vkUpdateDescriptorSets(
			logical_device->GetHandle(),
			static_cast<uint32_t>(with_writes.size()),
			with_writes.data(),
			0,
			nullptr
		);
	}

	void Pipeline::UpdateDescriptorSetsAll(size_t user_index, const VkWriteDescriptorSet& with_write)
	{
		per_frame_array<VkWriteDescriptorSet> writes;
		std::fill(writes.begin(), writes.end(), with_write);

		UpdateDescriptorSets(user_index, writes);
	}

	void Pipeline::BindPipeline(size_t user_index, VkCommandBuffer with_command_buffer, uint32_t current_frame)
	{
		vkCmdBindDescriptorSets(
			with_command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline_layout,
			0,
			1,
			&this->user_data[user_index].descriptor_sets[current_frame],
			0,
			nullptr
		);

		vkCmdBindPipeline(with_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->native_handle);
	}
} // namespace Engine::Rendering::Vulkan
