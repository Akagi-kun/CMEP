#include <assert.h>
#include <cstring>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Object.hpp"
#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/Texture.hpp"

#include "Engine.hpp"

namespace Engine::Rendering
{
	SpriteRenderer::SpriteRenderer(Engine* engine) : IRenderer(engine)
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings = renderer->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		// pipeline_settings.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		pipeline_settings.descriptorLayoutSettings.binding.push_back(1);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);

		this->pipeline = renderer->createVulkanPipeline(
			pipeline_settings,
			"game/shaders/vulkan/spriterenderer_vert.spv",
			"game/shaders/vulkan/spriterenderer_frag.spv"
		);
	}

	SpriteRenderer::~SpriteRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up sprite renderer");
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		vkDeviceWaitIdle(renderer->GetLogicalDevice());

		if (this->vbo != nullptr)
		{
			renderer->cleanupVulkanBuffer(this->vbo);
		}
		renderer->cleanupVulkanPipeline(this->pipeline);
	}

	void SpriteRenderer::Update(
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
		this->pos = pos;
		this->size = size;
		this->rotation = rotation;

		this->parent_pos = parent_position;
		this->parent_rotation = parent_rotation;
		this->parent_size = parent_size;

		this->screenx = screenx;
		this->screeny = screeny;

		this->has_updated_mesh = false;
	}

	void SpriteRenderer::SupplyData(RendererSupplyData data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::TEXTURE:
			{
				this->texture = std::static_pointer_cast<Texture>(data.payload_ptr);
				this->has_updated_mesh = false;
				return;
			}
		}

		throw std::runtime_error("Tried to supply Renderer data with payload type unsupported by the renderer!");
	}

	void SpriteRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->vbo == nullptr)
		{
			std::array<RenderingVertex, 6> vertices = {};
			vertices[0] = {glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0, 1.0)};
			vertices[1] = {glm::vec3(1.0, 1.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.0, 1.0)};
			vertices[2] = {glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0, 0.0)};
			vertices[3] = {glm::vec3(1.0, 1.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.0, 1.0)};
			vertices[4] = {glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.0, 0.0)};
			vertices[5] = {glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0, 0.0)};

			std::vector<RenderingVertex> generated_mesh{};
			generated_mesh.insert(generated_mesh.end(), vertices.begin(), vertices.end());

			this->vbo = renderer->createVulkanVertexBufferFromData(generated_mesh);
		}

		glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -10.0f, 10.0f);

		if (this->parent_size.x == 0.0f && this->parent_size.y == 0.0f && this->parent_size.z == 0.0f)
		{
			this->parent_size = glm::vec3(1, 1, 1);
		}

		glm::quat model_rotation = glm::quat(glm::radians(this->rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(this->parent_rotation));
		glm::mat4 model = glm::scale(
			glm::translate(
				glm::scale(
					glm::translate(glm::mat4(1.0f), this->parent_pos) * glm::mat4_cast(parent_rotation),
					this->parent_size
				),
				this->pos
			) * glm::mat4_cast(model_rotation),
			this->size
		);

		this->mat_mvp = projection * model;

		VulkanTextureImage* texture_image = this->texture->GetTextureImage();
		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = pipeline->uniformBuffers[i]->buffer;
			buffer_info.offset = 0;
			buffer_info.range = sizeof(glm::mat4);

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = texture_image->image->imageView;
			image_info.sampler = texture_image->textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = pipeline->vkDescriptorSets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = pipeline->vkDescriptorSets[i];
			descriptor_writes[1].dstBinding = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(
				renderer->GetLogicalDevice(),
				static_cast<uint32_t>(descriptor_writes.size()),
				descriptor_writes.data(),
				0,
				nullptr
			);
		}
	}

	void SpriteRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		vkMapMemory(
			renderer->GetLogicalDevice(),
			pipeline->uniformBuffers[currentFrame]->allocationInfo.deviceMemory,
			pipeline->uniformBuffers[currentFrame]->allocationInfo.offset,
			pipeline->uniformBuffers[currentFrame]->allocationInfo.size,
			0,
			&(pipeline->uniformBuffers[currentFrame]->mappedData)
		);

		memcpy(this->pipeline->uniformBuffers[currentFrame]->mappedData, &this->mat_mvp, sizeof(glm::mat4));
		vkUnmapMemory(
			renderer->GetLogicalDevice(), pipeline->uniformBuffers[currentFrame]->allocationInfo.deviceMemory
		);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline->vkPipelineLayout,
			0,
			1,
			&this->pipeline->vkDescriptorSets[currentFrame],
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