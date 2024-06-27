#include "Rendering/TextRenderer.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Engine::Rendering
{
	TextRenderer::TextRenderer(Engine* engine, IMeshBuilder* with_builder) : IRenderer(engine, with_builder)
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);
		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		this->pipeline = renderer->CreateVulkanPipeline(
			pipeline_settings,
			"game/shaders/vulkan/textrenderer_vert.spv",
			"game/shaders/vulkan/textrenderer_frag.spv"
		);
	}

	TextRenderer::~TextRenderer()
	{
		delete this->mesh_builder;
		this->mesh_builder = nullptr;

		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up text renderer");
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		vkDeviceWaitIdle(renderer->GetLogicalDevice());

		// if (this->vbo != nullptr)
		//{
		//	renderer->CleanupVulkanBuffer(this->vbo);
		// }
		renderer->CleanupVulkanPipeline(this->pipeline);
	}

	void TextRenderer::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				this->font			   = std::static_pointer_cast<Font>(data.payload_ptr);
				this->has_updated_mesh = false;
				break;
			}
			case RendererSupplyDataType::TEXT:
			{
				this->text.assign(data.payload_string);
				this->has_updated_mesh = false;
				break;
			}
			default:
			{
				throw std::runtime_error("Tried to supply Renderer data with payload type unsupported by the renderer!"
				);
			}
		}

		this->mesh_builder->SupplyData(data);
	}

	void TextRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		glm::mat4 projection{};
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			projection = locked_scene_manager->GetProjectionMatrixOrtho();
		}

		if (this->parent_transform.size.x == 0.0f || this->parent_transform.size.y == 0.0f ||
			this->parent_transform.size.z == 0.0f)
		{
			this->parent_transform.size = glm::vec3(1, 1, 1);
		}

		// Rotation quaternions
		glm::quat model_rotation  = glm::quat(glm::radians(this->transform.rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(this->parent_transform.rotation));

		glm::mat4 model = glm::scale(
			glm::translate(
				glm::scale(
					glm::translate(glm::mat4(1.0f), this->parent_transform.pos) * glm::mat4_cast(parent_rotation),
					this->parent_transform.size
				),
				this->transform.pos
			) * glm::mat4_cast(model_rotation),
			this->transform.size
		);

		this->mat_mvp = projection * model;

		if (this->mesh_builder != nullptr)
		{
			this->mesh_builder->Build();
			this->mesh_context = this->mesh_builder->GetContext();
		}

		auto page_texture	= this->font->GetPageTexture(0);
		this->texture_image = page_texture->GetTextureImage();

		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = pipeline->uniform_buffers[i]->buffer;
			buffer_info.offset = 0;
			buffer_info.range  = sizeof(glm::mat4);

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView   = this->texture_image->image->image_view;
			image_info.sampler	   = this->texture_image->texture_sampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet			 = pipeline->vk_descriptor_sets[i];
			descriptor_writes[0].dstBinding		 = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo	 = &buffer_info;

			descriptor_writes[1].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet			 = pipeline->vk_descriptor_sets[i];
			descriptor_writes[1].dstBinding		 = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo		 = &image_info;

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
		if (this->text.empty())
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
			pipeline->uniform_buffers[currentFrame]->allocation_info.deviceMemory,
			pipeline->uniform_buffers[currentFrame]->allocation_info.offset,
			pipeline->uniform_buffers[currentFrame]->allocation_info.size,
			0,
			&(pipeline->uniform_buffers[currentFrame]->mapped_data)
		);

		memcpy(this->pipeline->uniform_buffers[currentFrame]->mapped_data, &this->mat_mvp, sizeof(glm::mat4));

		vkUnmapMemory(
			renderer->GetLogicalDevice(),
			pipeline->uniform_buffers[currentFrame]->allocation_info.deviceMemory
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
		VkBuffer vertex_buffers[] = {this->mesh_context.vbo->buffer};
		VkDeviceSize offsets[]	  = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(this->mesh_context.vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
