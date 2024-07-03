#include "Rendering/MeshRenderer.hpp"

#include "Assets/Texture.hpp"
#include "Rendering/Vulkan/VulkanImage.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "glm/trigonometric.hpp"

#include <cassert>
#include <cstring>
#include <glm/gtc/quaternion.hpp>

namespace Engine::Rendering
{
	MeshRenderer::MeshRenderer(Engine* engine) : IRenderer(engine, nullptr)
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.shader = {"game/shaders/vulkan/mesh_vert.spv", "game/shaders/vulkan/mesh_frag.spv"};

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);

		// TODO: Fix descriptor count
		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			16,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		this->pipeline = renderer->CreateVulkanPipeline(pipeline_settings);

		this->mat_mvp = glm::mat4();
	}

	MeshRenderer::~MeshRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up mesh renderer");

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		renderer->SyncDeviceWaitIdle();
		// vkDeviceWaitIdle(renderer->GetLogicalDevice());

		if (this->vbo != nullptr)
		{
			renderer->CleanupVulkanBuffer(this->vbo);
		}
		renderer->CleanupVulkanPipeline(this->pipeline);
	}

	void MeshRenderer::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::TEXTURE:
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
			}
			default:
				break;
		}

		throw std::runtime_error("Tried to supply Renderer data with payload type unsupported by the renderer!");
	}

	void MeshRenderer::UpdateMesh()
	{
		if (!this->mesh)
		{
			return;
		}

		this->has_updated_mesh = true;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		/*
				glm::mat4 projection = glm::perspective<float>(
					glm::radians(45.0f),
					static_cast<float>(this->screen.x / this->screen.y),
					0.1f,
					100.0f
				); */

		glm::mat4 projection;
		glm::mat4 view;
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

		glm::quat model_rotation  = glm::quat(glm::radians(this->transform.rotation));
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
		  );

		// this->mat_m		 = model;
		// this->mat_v		 = view;
		// this->mat_mv	 = view * model;
		// this->mat_mv_3x3 = glm::mat3(view * model);
		this->mat_mvp = projection * view * model;

		if (!this->has_updated_meshdata)
		{
			this->has_updated_meshdata = true;

			if (this->vbo != nullptr)
			{
				renderer->SyncDeviceWaitIdle();
				// vkDeviceWaitIdle(renderer->GetLogicalDevice());
				renderer->CleanupVulkanBuffer(this->vbo);
				this->vbo = nullptr;
			}

			std::vector<RenderingVertex> generated_mesh = {};

			generated_mesh.resize(this->mesh->mesh_vertices.size());

			uint32_t vertex_idx = 0;
			for (glm::vec3 pos : this->mesh->mesh_vertices)
			{
				generated_mesh[vertex_idx].pos	 = pos;
				generated_mesh[vertex_idx].color = glm::vec3(0);
				vertex_idx++;
			}

			vertex_idx = 0;
			for (glm::vec2 tex_coord : this->mesh->mesh_uvs)
			{
				generated_mesh[vertex_idx].texcoord = tex_coord;
				vertex_idx++;
			}

			vertex_idx = 0;
			for (glm::vec3 normal : this->mesh->mesh_normals)
			{
				generated_mesh[vertex_idx].normal = normal;
				vertex_idx++;
			}

			this->vbo = renderer->CreateVulkanVertexBufferFromData(generated_mesh);

			this->vbo_vert_count = generated_mesh.size();

			std::vector<VkDescriptorImageInfo> diffuse_image_buffer_infos{};
			diffuse_image_buffer_infos.resize(this->mesh->diffuse_textures.size());
			for (size_t diffuse_texture_index = 0; diffuse_texture_index < this->mesh->diffuse_textures.size();
				 diffuse_texture_index++)
			{
				// Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Filling imageBufferInfos at index %u",
				// diffuseTextureIndex);

				if (this->mesh->diffuse_textures[diffuse_texture_index] != nullptr)
				{
					VulkanTextureImage* current_diffuse_texture_image =
						this->mesh->diffuse_textures[diffuse_texture_index]->GetTextureImage();

					if (current_diffuse_texture_image != nullptr)
					{
						if (current_diffuse_texture_image->image != nullptr ||
							current_diffuse_texture_image->texture_sampler != VK_NULL_HANDLE)
						{
							VkDescriptorImageInfo diffuse_image_buffer_info{};
							diffuse_image_buffer_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							diffuse_image_buffer_info.imageView	  = current_diffuse_texture_image->image_view;
							diffuse_image_buffer_info.sampler	  = current_diffuse_texture_image->texture_sampler;
							diffuse_image_buffer_infos[diffuse_texture_index] = diffuse_image_buffer_info;
						}
					}
				}
			}

			if (auto locked_device_manager = renderer->GetDeviceManager().lock())
			{
				for (size_t i = 0; i < VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
				{
					VkDescriptorBufferInfo uniform_buffer_info{};
					uniform_buffer_info.buffer = pipeline->uniform_buffers[i]->buffer;
					uniform_buffer_info.offset = 0;
					uniform_buffer_info.range  = sizeof(glm::mat4);

					std::vector<VkWriteDescriptorSet> descriptor_writes{};
					descriptor_writes.resize(1);

					descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptor_writes[0].dstSet			 = pipeline->vk_descriptor_sets[i];
					descriptor_writes[0].dstBinding		 = 0;
					descriptor_writes[0].dstArrayElement = 0;
					descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptor_writes[0].descriptorCount = 1;
					descriptor_writes[0].pBufferInfo	 = &uniform_buffer_info;

					if (diffuse_image_buffer_infos.empty())
					{
						this->logger->SimpleLog(
							Logging::LogLevel::Debug3,
							"Updating set 0x%x binding 1",
							pipeline->vk_descriptor_sets[i]
						);

						descriptor_writes.resize(2);
						descriptor_writes[1].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						descriptor_writes[1].dstSet			 = pipeline->vk_descriptor_sets[i];
						descriptor_writes[1].dstBinding		 = 1;
						descriptor_writes[1].dstArrayElement = 0;
						descriptor_writes[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						descriptor_writes[1].descriptorCount = static_cast<uint32_t>(diffuse_image_buffer_infos.size());
						descriptor_writes[1].pImageInfo		 = diffuse_image_buffer_infos.data();
					}
					this->logger->SimpleLog(
						Logging::LogLevel::Debug3,
						"Descriptor set 0x%x write of index 1 has binding %u, descriptorWrite size is %u",
						pipeline->vk_descriptor_sets[i],
						descriptor_writes[1].dstBinding,
						static_cast<unsigned int>(descriptor_writes.size())
					);

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
	}

	void MeshRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		VulkanUtils::VulkanUniformBufferTransfer(
			renderer,
			this->pipeline,
			currentFrame,
			&this->mat_mvp,
			sizeof(glm::mat4)
		);
		/*
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
 */
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
		VkDeviceSize offsets[]	  = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(this->vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
