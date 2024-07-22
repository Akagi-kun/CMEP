#include "Rendering/Renderer3D.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"
#include "Rendering/framework.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"

#include <cassert>
#include <glm/gtc/quaternion.hpp>

namespace Engine::Rendering
{
	Renderer3D::Renderer3D(
		Engine* engine,
		IMeshBuilder* with_builder,
		const char* with_pipeline_program,
		VkPrimitiveTopology with_primitives
	)
		: IRenderer(engine, with_builder, with_pipeline_program, with_primitives)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = with_primitives;

		pipeline_settings.shader = this->pipeline_name;

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);

		// TODO: Fix descriptor count
		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		this->pipeline =
			new Vulkan::VPipeline(renderer->GetDeviceManager(), pipeline_settings, renderer->GetRenderPass());

		this->mat_mvp = glm::mat4();
	}

	Renderer3D::~Renderer3D()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up mesh renderer");

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		renderer->SyncDeviceWaitIdle();

		// delete this->vbo;

		delete this->pipeline;
	}

	void Renderer3D::SupplyData(const RendererSupplyData& data)
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
			/* case RendererSupplyDataType::TEXTURE:
			{
				this->texture			   = std::static_pointer_cast<Texture>(data.payload_ptr);
				this->has_updated_mesh	   = false;
				this->has_updated_meshdata = false;
				return;
			}
			 case RendererSupplyDataType::MESH:
			{
				this->mesh				   = std::static_pointer_cast<Mesh>(data.payload_ptr);
				this->has_updated_mesh	   = false;
				this->has_updated_meshdata = false;
				return;
			} */
			default:
			{
				break;
			}
		}

		assert(this->mesh_builder != nullptr && "This renderer has not been assigned a mesh builder!");
		this->mesh_builder->SupplyData(data);
	}

	void Renderer3D::UpdateMesh()
	{
		/* if (!this->mesh)
		{
			return;
		} */

		this->has_updated_mesh = true;

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->mesh_builder != nullptr)
		{
			this->mesh_builder->Build();
		}

		glm::mat4 view;
		glm::mat4 projection;
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			view	   = locked_scene_manager->GetCameraViewMatrix();
			projection = locked_scene_manager->GetProjectionMatrix(this->screen);
		}
		projection[1][1] *= -1;

		if (this->parent_transform.size.x == 0.0f && this->parent_transform.size.y == 0.0f &&
			this->parent_transform.size.z == 0.0f)
		{
			this->parent_transform.size = glm::vec3(1, 1, 1);
		}

		glm::mat4 model = CalculateModelMatrix(this->transform, this->parent_transform);

		this->mat_mvp = projection * view; // * model;

		// auto* texture_image = this->texture->GetTextureImage();

		if (auto* device_manager = renderer->GetDeviceManager())
		{
			for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				VkDescriptorBufferInfo buffer_info{};
				buffer_info.buffer = this->pipeline->GetUniformBuffer(i)->GetNativeHandle();
				buffer_info.offset = 0;
				buffer_info.range  = sizeof(glm::mat4);

				/* VkDescriptorImageInfo image_info{};
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView   = texture_image->GetNativeViewHandle();
				image_info.sampler	   = texture_image->texture_sampler; */

				std::array<VkWriteDescriptorSet, 1> descriptor_writes{};

				descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[0].dstSet			 = this->pipeline->GetDescriptorSet(i);
				descriptor_writes[0].dstBinding		 = 0;
				descriptor_writes[0].dstArrayElement = 0;
				descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptor_writes[0].descriptorCount = 1;
				descriptor_writes[0].pBufferInfo	 = &buffer_info;

				/* descriptor_writes[1].sType		 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[1].dstSet			 = pipeline->GetDescriptorSet(i);
				descriptor_writes[1].dstBinding		 = 1;
				descriptor_writes[1].dstArrayElement = 0;
				descriptor_writes[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_writes[1].descriptorCount = 1;
				descriptor_writes[1].pImageInfo		 = &image_info; */

				vkUpdateDescriptorSets(
					device_manager->GetLogicalDevice(),
					static_cast<uint32_t>(descriptor_writes.size()),
					descriptor_writes.data(),
					0,
					nullptr
				);
			}
		}
	}

	void Renderer3D::Render(VkCommandBuffer command_buffer, uint32_t current_frame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		if (this->mesh_builder->HasRebuilt())
		{
			this->mesh_context = this->mesh_builder->GetContext();
		}

		Vulkan::Utils::VulkanUniformBufferTransfer(this->pipeline, current_frame, &this->mat_mvp, sizeof(glm::mat4));

		this->pipeline->BindPipeline(command_buffer, current_frame);

		VkBuffer vertex_buffers[] = {this->mesh_context.vbo->GetNativeHandle()};
		VkDeviceSize offsets[]	  = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(command_buffer, static_cast<uint32_t>(this->mesh_context.vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
