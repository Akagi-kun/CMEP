#include "Rendering/AxisRenderer.hpp"

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "glm/ext/matrix_clip_space.hpp"

#include <cassert>
#include <cstring>

namespace Engine::Rendering
{
	AxisRenderer::AxisRenderer(Engine* engine) : IRenderer(engine)
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings = renderer->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		this->pipeline = renderer->createVulkanPipeline(
			pipeline_settings, "game/shaders/vulkan/axisrenderer_vert.spv", "game/shaders/vulkan/axisrenderer_frag.spv"
		);
	}

	AxisRenderer::~AxisRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up axis renderer");

		Rendering::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		vkDeviceWaitIdle(renderer->GetLogicalDevice());

		renderer->cleanupVulkanBuffer(this->vbo);
		renderer->cleanupVulkanPipeline(this->pipeline);
	}
	/*
		void AxisRenderer::Update(
			glm::vec3 pos,
			glm::vec3 size,
			glm::vec3 rotation,
			uint_fast16_t screenx,
			uint_fast16_t screeny,
			glm::vec3 parent_position,
			glm::vec3 parent_rotation,
			glm::vec3 parent_size
		)
		{
			this->transform.pos = pos;
			this->transform.size = size;
			this->transform.rotation = rotation;

			this->parent_transform.pos = parent_position;
			this->parent_transform.rotation = parent_rotation;
			this->parent_transform.size = parent_size;

			this->screenx = screenx;
			this->screeny = screeny;

			this->has_updated_mesh = false;
		}
	 */
	void AxisRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		const std::vector<RenderingVertex> vertices = {
			{{0.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}},
			{{1.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}},
			{{0.0, 0.0, 0.0}, {0.0f, 0.0f, 1.0f}},
			{{0.0, 1.0, 0.0}, {0.0f, 0.0f, 1.0f}},
			{{0.0, 0.0, 0.0}, {1.0f, 0.0f, 0.0f}},
			{{0.0, 0.0, 1.0}, {1.0f, 0.0f, 0.0f}}
		};

		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f), static_cast<float>(this->screen.x / this->screen.y), 0.1f, 100.0f
		);

		glm::mat4 view;
		if (auto locked_scene_manager = this->scene_manager.lock())
		{
			view = locked_scene_manager->GetCameraViewMatrix();
		}
		glm::mat4 model = glm::mat4(1.0f);

		projection[1][1] *= -1;

		this->mat_mvp = projection * view * model;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->vbo == nullptr)
		{
			this->vbo = renderer->createVulkanVertexBufferFromData(vertices);
		}

		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = pipeline->uniform_buffers[i]->buffer;
			buffer_info.offset = 0;
			buffer_info.range = sizeof(glm::mat4);

			std::vector<VkWriteDescriptorSet> descriptor_writes{};
			descriptor_writes.resize(1);

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = pipeline->vk_descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			vkUpdateDescriptorSets(
				renderer->GetLogicalDevice(),
				static_cast<uint32_t>(descriptor_writes.size()),
				descriptor_writes.data(),
				0,
				nullptr
			);
		}
	}

	void AxisRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		vkMapMemory(
			renderer->GetLogicalDevice(),
			pipeline->uniform_buffers[currentFrame]->allocationInfo.deviceMemory,
			pipeline->uniform_buffers[currentFrame]->allocationInfo.offset,
			pipeline->uniform_buffers[currentFrame]->allocationInfo.size,
			0,
			&(pipeline->uniform_buffers[currentFrame]->mappedData)
		);

		memcpy(this->pipeline->uniform_buffers[currentFrame]->mappedData, &this->mat_mvp, sizeof(glm::mat4));
		vkUnmapMemory(
			renderer->GetLogicalDevice(), pipeline->uniform_buffers[currentFrame]->allocationInfo.deviceMemory
		);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline->vk_pipeline_layout,
			0,
			1,
			&this->pipeline->vk_descriptor_sets[currentFrame],
			0,
			nullptr
		);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
		VkBuffer vertex_buffers[] = {this->vbo->buffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
} // namespace Engine::Rendering
