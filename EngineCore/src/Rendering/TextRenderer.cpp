#include <array>
#include <assert.h>
#include <cstring>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Logging/Logging.hpp"
#include "Rendering/Font.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/Texture.hpp"

#include "Engine.hpp"

namespace Engine::Rendering
{
	TextRenderer::TextRenderer(Engine* engine) : IRenderer(engine)
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings = renderer->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		pipeline_settings.descriptorLayoutSettings.binding.push_back(1);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);

		this->pipeline = renderer->createVulkanPipeline(
			pipeline_settings, "game/shaders/vulkan/textrenderer_vert.spv", "game/shaders/vulkan/textrenderer_frag.spv"
		);
	}

	TextRenderer::~TextRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up text renderer");
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		vkDeviceWaitIdle(renderer->GetLogicalDevice());

		if (this->vbo != nullptr)
		{
			renderer->cleanupVulkanBuffer(this->vbo);
		}
		renderer->cleanupVulkanPipeline(this->pipeline);
	}

	void TextRenderer::Update(
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

	void TextRenderer::SupplyData(RendererSupplyData data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				this->font = std::static_pointer_cast<Font>(data.payload_ptr);
				this->has_updated_mesh = false;
				return;
			}
			case RendererSupplyDataType::TEXT:
			{
				this->text.assign(data.payload_string);
				this->has_updated_mesh = false;
				return;
			}
			default:
			{
				break;
			}
		}

		throw std::runtime_error("Tried to supply Renderer data with payload type unsupported by the renderer!");
	}

	void TextRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->vbo != nullptr)
		{
			vkDeviceWaitIdle(renderer->GetLogicalDevice());
			renderer->cleanupVulkanBuffer(this->vbo);
			this->vbo = nullptr;
		}

		int fontsize = std::stoi(*this->font->GetFontInfoParameter("size"), nullptr, 10);

		assert(fontsize > 0);

		std::vector<RenderingVertex> generated_mesh = {};

		float accu_x = 0.f;
		float accu_y = 0.f;
		for (size_t i = 0; i < this->text.size(); i++)
		{
			vbo = 0;
			if (this->text[i] == '\n')
			{
				accu_y -= 0.35f * (float)(std::round(this->size.y) / fontsize);
				accu_x = 0.f;
			}
			else if (this->text[i] == ' ')
			{
				accu_x += 0.05f * (float)(std::round(this->size.x) / fontsize);
			}
			else
			{
				// Get character info
				Rendering::FontChar* ch = this->font->GetChar(this->text[i]);
				assert(ch != nullptr);
				if (ch == nullptr)
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Error, "Char 0x%x is not found in set font", this->text[i]
					);
					continue;
				}

				// Get texture
				unsigned int texture_x = 0, texture_y = 0;
				std::shared_ptr<Texture> texture = this->font->GetPageTexture(ch->page);
				assert(texture != nullptr);
				texture->GetSize(texture_x, texture_y);
				assert(texture_x > 0 && texture_y > 0);

				// Obscure math I don't understand, achieved with trial and error and works so just leave it like this
				// const float xs = ch->width / (float)this->_screenx * 2 * (float)(std::round(_size.x) / fontsize);
				// const float ys = ch->height / (float)this->_screeny * 2 * (float)(std::round(_size.y) / fontsize);
				// const float x = (float)this->_pos.x * 2 - 1.f + accu_x;
				// const float y = (float)this->_pos.y * 2 - 1.f + accu_y;

				const float xs = ch->width / (float)this->screenx * (float)(std::round(size.x) / fontsize);
				const float ys = ch->height / (float)this->screeny * (float)(std::round(size.y) / fontsize);
				const float x = accu_x;
				const float y = accu_y;
				const float z = 0.0f;

				std::array<RenderingVertex, 6> vertices = {};
				vertices[0] = {
					glm::vec3(x, ys + y, z),
					glm::vec3(1.f, 1.f, 1.f),
					glm::vec2((ch->x) / (float)texture_x, (ch->y + ch->height) / (float)texture_y)
				};
				vertices[1] = {
					glm::vec3(xs + x, ys + y, z),
					glm::vec3(1.f, 1.f, 1.f),
					glm::vec2((ch->x + ch->width) / (float)texture_x, (ch->y + ch->height) / (float)texture_y)
				};
				vertices[2] = {
					glm::vec3(x, y, z),
					glm::vec3(1.f, 1.f, 1.f),
					glm::vec2((ch->x) / (float)texture_x, (ch->y) / (float)texture_y)
				};
				vertices[3] = {
					glm::vec3(xs + x, ys + y, z),
					glm::vec3(1.f, 1.f, 1.f),
					glm::vec2((ch->x + ch->width) / (float)texture_x, (ch->y + ch->height) / (float)texture_y)
				};
				vertices[4] = {
					glm::vec3(xs + x, y, z),
					glm::vec3(1.f, 1.f, 1.f),
					glm::vec2((ch->x + ch->width) / (float)texture_x, (ch->y) / (float)texture_y)
				};
				vertices[5] = {
					glm::vec3(x, y, z),
					glm::vec3(1.f, 1.f, 1.f),
					glm::vec2((ch->x) / (float)texture_x, (ch->y) / (float)texture_y)
				};

				accu_x += xs + (2.f / this->screenx);

				generated_mesh.insert(generated_mesh.end(), vertices.begin(), vertices.end());

				this->texture_image = texture->GetTextureImage();
			}
		}

		glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -10.0f, 10.0f);

		if (this->parent_size.x == 0.0f && this->parent_size.y == 0.0f && this->parent_size.z == 0.0f)
		{
			this->parent_size = glm::vec3(1, 1, 1);
		}

		glm::quat model_rotation = glm::quat(glm::radians(this->rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(this->parent_rotation));
		glm::mat4 model = glm::translate(
							  glm::translate(glm::mat4(1.0f), this->parent_pos) * glm::mat4_cast(parent_rotation),
							  this->pos
						  ) *
						  glm::mat4_cast(model_rotation);

		this->mat_mvp = projection * model;

		assert(generated_mesh.size() > 0);

		this->vbo_vert_count = generated_mesh.size();

		this->vbo = renderer->createVulkanVertexBufferFromData(generated_mesh);

		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = pipeline->uniform_buffers[i]->buffer;
			buffer_info.offset = 0;
			buffer_info.range = sizeof(glm::mat4);

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = this->texture_image->image->imageView;
			image_info.sampler = this->texture_image->textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = pipeline->vk_descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = pipeline->vk_descriptor_sets[i];
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

		// renderer->updateVulkanDescriptorSetsVulkanTextureImage(this->pipeline, this->textureImage);
	}

	void TextRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (this->text.size() == 0)
		{
			return;
		}

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

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(this->vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
