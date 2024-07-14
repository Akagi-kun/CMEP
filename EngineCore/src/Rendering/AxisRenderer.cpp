#include "Rendering/AxisRenderer.hpp"

#include "Rendering/IMeshBuilder.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"

#include <cassert>
#include <cstring>
#include <string>

namespace Engine::Rendering
{
	AxisRenderer::AxisRenderer(Engine* engine, IMeshBuilder* with_builder) : IRenderer(engine, with_builder)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		std::string with_program_name = "axis";
		pipeline_settings.shader	  = {
			 with_program_name + "_vert.spv",
			 with_program_name + "_frag.spv",
		 };

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);

		this->pipeline = renderer->CreateVulkanPipeline(pipeline_settings);
	}

	AxisRenderer::~AxisRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up axis renderer");

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		renderer->SyncDeviceWaitIdle();

		delete this->vbo;

		renderer->CleanupVulkanPipeline(this->pipeline);
	}

	void AxisRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		if (this->mesh_builder != nullptr)
		{
			this->mesh_builder->Build();
		}

		glm::mat4 projection;
		glm::mat4 view;
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			view	   = locked_scene_manager->GetCameraViewMatrix();
			projection = locked_scene_manager->GetProjectionMatrix(this->screen);
		}
		projection[1][1] *= -1;

		// auto model = glm::mat4(1.0f);

		this->mat_mvp = projection * view; // * model;

		if (auto locked_device_manager = renderer->GetDeviceManager().lock())
		{
			for (size_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				VkDescriptorBufferInfo buffer_info{};
				buffer_info.buffer = pipeline->uniform_buffers[i]->GetNativeHandle();
				buffer_info.offset = 0;
				buffer_info.range  = sizeof(glm::mat4);

				std::vector<VkWriteDescriptorSet> descriptor_writes{};
				descriptor_writes.resize(1);

				descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[0].dstSet			 = this->pipeline->vk_descriptor_sets[i];
				descriptor_writes[0].dstBinding		 = 0;
				descriptor_writes[0].dstArrayElement = 0;
				descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptor_writes[0].descriptorCount = 1;
				descriptor_writes[0].pBufferInfo	 = &buffer_info;

				vkUpdateDescriptorSets(
					locked_device_manager->GetLogicalDevice(),
					static_cast<uint32_t>(descriptor_writes.size()),
					descriptor_writes.data(),
					0,
					nullptr
				);
			}
		}
	}

	void AxisRenderer::Render(VkCommandBuffer command_buffer, uint32_t current_frame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		if (this->mesh_builder->HasRebuilt())
		{
			this->mesh_context = this->mesh_builder->GetContext();
		}

		// Skip render if VBO empty
		if (this->mesh_context.vbo_vert_count <= 0)
		{
			return;
		}

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		Vulkan::Utils::VulkanUniformBufferTransfer(
			renderer,
			this->pipeline,
			current_frame,
			&this->mat_mvp,
			sizeof(glm::mat4)
		);

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline->vk_pipeline_layout,
			0,
			1,
			&this->pipeline->vk_descriptor_sets[current_frame],
			0,
			nullptr
		);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
		VkBuffer vertex_buffers[] = {this->mesh_context.vbo->GetNativeHandle()};
		VkDeviceSize offsets[]	  = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(command_buffer, static_cast<uint32_t>(this->mesh_context.vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
