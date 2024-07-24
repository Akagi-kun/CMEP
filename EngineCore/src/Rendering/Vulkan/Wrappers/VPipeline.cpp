#include "Rendering/Vulkan/Wrappers/VPipeline.hpp"

#include "Rendering/IRenderer.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVMA.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"
#include "Rendering/Vulkan/Wrappers/VShaderModule.hpp"

#include "Engine.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	VPipeline::VPipeline(
		VDeviceManager* with_device_manager,
		VulkanPipelineSettings& settings,
		VkRenderPass with_render_pass
	)
		: HoldsVulkanDevice(with_device_manager), HoldsVMA(with_device_manager->GetVmaAllocator())
	{
		const auto& logical_device = this->device_manager->GetLogicalDevice();

		std::string shader_path = this->device_manager->GetOwnerEngine()->GetShaderPath();

		assert(!settings.shader.empty() && "A valid shader for this pipeline is required!");

		// Vertex stage
		auto vert_shader_module = VShaderModule(this->device_manager, shader_path, settings.shader + "_vert.spv");

		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module.GetNativeHandle();
		vert_shader_stage_info.pName  = "main";

		// Fragment stage
		auto frag_shader_module = VShaderModule(this->device_manager, shader_path, settings.shader + "_frag.spv");

		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module.GetNativeHandle();
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

		std::vector<VkDescriptorSetLayoutBinding> bindings	= {};
		std::vector<VkDescriptorBindingFlags> binding_flags = {};

		for (auto& setting : settings.descriptor_layout_settings)
		{
			VkDescriptorSetLayoutBinding new_binding{};
			new_binding.binding			   = setting.binding;
			new_binding.descriptorCount	   = setting.descriptor_count;
			new_binding.descriptorType	   = setting.types;
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
				this->device_manager->GetLogicalDevice(),
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

		if (vkCreatePipelineLayout(logical_device, &pipeline_layout_info, nullptr, &(this->pipeline_layout)) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType		 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports	 = &settings.viewport;
		viewport_state.scissorCount	 = 1;
		viewport_state.pScissors	 = &settings.scissor;

		settings.color_blending.pAttachments = &settings.color_blend_attachment;

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType				  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount		  = 2;
		pipeline_info.pStages			  = shader_stages;
		pipeline_info.pVertexInputState	  = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &settings.input_assembly;
		pipeline_info.pViewportState	  = &viewport_state;
		pipeline_info.pRasterizationState = &settings.rasterizer;
		pipeline_info.pMultisampleState	  = &settings.multisampling;
		pipeline_info.pDepthStencilState  = nullptr; // Optional
		pipeline_info.pColorBlendState	  = &settings.color_blending;
		pipeline_info.pDynamicState		  = &dynamic_state;
		pipeline_info.layout			  = this->pipeline_layout;
		pipeline_info.pDepthStencilState  = &settings.depth_stencil;
		pipeline_info.renderPass		  = with_render_pass;
		pipeline_info.subpass			  = 0;
		pipeline_info.basePipelineHandle  = VK_NULL_HANDLE; // Optional
		pipeline_info.basePipelineIndex	  = -1;				// Optional

		if (vkCreateGraphicsPipelines(
				logical_device,
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
		// this->CreateVulkanUniformBuffers(pipeline);

		VkDeviceSize buffer_size = sizeof(RendererMatrixData);

		this->uniform_buffers.resize(VulkanRenderingEngine::GetMaxFramesInFlight());

		for (size_t i = 0; i < VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
		{
			this->uniform_buffers[i] = new VBuffer(
				this->device_manager,
				buffer_size,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				0
			);
		}
		/************************************/

		/************************************/
		// this->CreateVulkanDescriptorPool(pipeline, settings.descriptor_layout_settings);

		std::vector<VkDescriptorPoolSize> pool_sizes{};

		pool_sizes.resize(settings.descriptor_layout_settings.size());
		for (size_t i = 0; i < settings.descriptor_layout_settings.size(); i++)
		{
			VkDescriptorPoolSize pool_size{};
			pool_size.type			  = settings.descriptor_layout_settings[i].types;
			pool_size.descriptorCount = VulkanRenderingEngine::GetMaxFramesInFlight() *
										settings.descriptor_layout_settings[i].descriptor_count;

			pool_sizes[i] = pool_size;
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes	= pool_sizes.data();
		pool_info.maxSets		= VulkanRenderingEngine::GetMaxFramesInFlight();

		if (vkCreateDescriptorPool(
				this->device_manager->GetLogicalDevice(),
				&pool_info,
				nullptr,
				&(this->descriptor_pool)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
		/************************************/

		/************************************/
		// this->CreateVulkanDescriptorSets(pipeline);

		std::vector<VkDescriptorSetLayout> layouts(
			VulkanRenderingEngine::GetMaxFramesInFlight(),
			this->descriptor_set_layout
		);

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool	  = this->descriptor_pool;
		alloc_info.descriptorSetCount = VulkanRenderingEngine::GetMaxFramesInFlight();
		alloc_info.pSetLayouts		  = layouts.data();

		this->descriptor_sets.resize(VulkanRenderingEngine::GetMaxFramesInFlight());

		VkResult create_result = vkAllocateDescriptorSets(
			this->device_manager->GetLogicalDevice(),
			&alloc_info,
			this->descriptor_sets.data()
		);

		if (create_result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		/************************************/
	}

	VPipeline::~VPipeline()
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		vkDeviceWaitIdle(logical_device);

		for (auto& uniform_buffer : this->uniform_buffers)
		{
			delete uniform_buffer;
		}

		vkDestroyDescriptorPool(logical_device, this->descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(logical_device, this->descriptor_set_layout, nullptr);

		vkDestroyPipeline(logical_device, this->native_handle, nullptr);
		vkDestroyPipelineLayout(logical_device, this->pipeline_layout, nullptr);
	}

	void VPipeline::BindPipeline(VkCommandBuffer with_command_buffer, uint32_t current_frame)
	{
		vkCmdBindDescriptorSets(
			with_command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline_layout,
			0,
			1,
			&this->descriptor_sets[current_frame],
			0,
			nullptr
		);

		vkCmdBindPipeline(with_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->native_handle);
	}
} // namespace Engine::Rendering::Vulkan
