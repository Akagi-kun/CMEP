#include "Rendering/TextRenderer.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"
#include "Rendering/framework.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Engine::Rendering
{
	TextRenderer::TextRenderer(Engine* engine, IMeshBuilder* with_builder) : IRenderer(engine, with_builder)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		std::string with_program_name = "text";
		pipeline_settings.shader	  = ShaderDefinition{
			 with_program_name + "_vert.spv",
			 with_program_name + "_frag.spv",
		 };

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);
		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		this->pipeline = renderer->CreateVulkanPipeline(pipeline_settings);
	}

	TextRenderer::~TextRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up text renderer");

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		renderer->SyncDeviceWaitIdle();
		renderer->CleanupVulkanPipeline(this->pipeline);
	}

	void TextRenderer::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				auto font_cast		   = std::static_pointer_cast<Font>(data.payload_ptr);
				this->texture		   = font_cast->GetPageTexture(0);
				this->has_updated_mesh = false;
				break;
			}
			case RendererSupplyDataType::TEXTURE:
			{
				this->texture		   = std::static_pointer_cast<Texture>(data.payload_ptr);
				this->has_updated_mesh = false;
				return;
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

		assert(this->mesh_builder != nullptr && "This renderer has not been assigned a mesh builder!");
		this->mesh_builder->SupplyData(data);
	}

	void TextRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->mesh_builder != nullptr)
		{
			this->mesh_builder->Build();
		}

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
		/* glm::quat model_rotation  = glm::quat(glm::radians(this->transform.rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(this->parent_transform.rotation));
		glm::mat4 model			  = glm::scale(
			  glm::translate(
				  glm::scale(
					  glm::translate(glm::mat4(1.0f), this->parent_transform.pos) * glm::mat4_cast(parent_rotation),
					  this->parent_transform.size
				  ),
				  this->transform.pos
			  ) * glm::mat4_cast(model_rotation),
			  this->transform.size
		  ); */

		glm::mat4 model = CalculateModelMatrix(this->transform, this->parent_transform);

		this->mat_mvp = projection * model;

		auto* texture_image = this->texture->GetTextureImage();

		if (auto locked_device_manager = renderer->GetDeviceManager().lock())
		{
			for (size_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				VkDescriptorBufferInfo buffer_info{};
				buffer_info.buffer = pipeline->uniform_buffers[i]->GetNativeHandle();
				buffer_info.offset = 0;
				buffer_info.range  = sizeof(glm::mat4);

				VkDescriptorImageInfo image_info{};
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView   = texture_image->image_view;
				image_info.sampler	   = texture_image->texture_sampler;

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
					locked_device_manager->GetLogicalDevice(),
					static_cast<uint32_t>(descriptor_writes.size()),
					descriptor_writes.data(),
					0,
					nullptr
				);
			}
		}
	}

	void TextRenderer::Render(VkCommandBuffer command_buffer, uint32_t current_frame)
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
