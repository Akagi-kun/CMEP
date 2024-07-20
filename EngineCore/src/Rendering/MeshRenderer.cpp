#include "Rendering/MeshRenderer.hpp"

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
	MeshRenderer::MeshRenderer(Engine* engine, const char* with_pipeline_program)
		: IRenderer(engine, nullptr, with_pipeline_program)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		std::string with_program_name = "mesh";
		pipeline_settings.shader	  = {
			 with_program_name + "_vert.spv",
			 with_program_name + "_frag.spv",
		 };

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

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		renderer->SyncDeviceWaitIdle();

		delete this->vbo;

		delete this->pipeline;
	}

	void MeshRenderer::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			/* case RendererSupplyDataType::TEXTURE:
			{
				this->texture			   = std::static_pointer_cast<Texture>(data.payload_ptr);
				this->has_updated_mesh	   = false;
				this->has_updated_meshdata = false;
				return;
			} */
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

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

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

		this->mat_mvp = projection * view * model;

		if (!this->has_updated_meshdata)
		{
			this->has_updated_meshdata = true;

			if (this->vbo != nullptr)
			{
				renderer->SyncDeviceWaitIdle();
				delete this->vbo;
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

			/* std::vector<VkDescriptorImageInfo> diffuse_image_buffer_infos{};
			diffuse_image_buffer_infos.resize(this->mesh->diffuse_textures.size());
			for (size_t diffuse_texture_index = 0; diffuse_texture_index < this->mesh->diffuse_textures.size();
				 diffuse_texture_index++)
			{
				// Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Filling imageBufferInfos at index %u",
				// diffuseTextureIndex);

				if (this->mesh->diffuse_textures[diffuse_texture_index] != nullptr)
				{
					Vulkan::VSampledImage* current_diffuse_texture_image =
						this->mesh->diffuse_textures[diffuse_texture_index]->GetTextureImage();

					if (current_diffuse_texture_image != nullptr)
					{
						if (current_diffuse_texture_image->GetNativeHandle() != nullptr ||
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
			} */

			if (auto locked_device_manager = renderer->GetDeviceManager().lock())
			{
				for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
				{
					VkDescriptorBufferInfo uniform_buffer_info{};
					uniform_buffer_info.buffer = pipeline->GetUniformBuffer(i)->GetNativeHandle();
					uniform_buffer_info.offset = 0;
					uniform_buffer_info.range  = sizeof(glm::mat4);

					std::vector<VkWriteDescriptorSet> descriptor_writes{};
					descriptor_writes.resize(1);

					descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptor_writes[0].dstSet			 = pipeline->GetDescriptorSet(i);
					descriptor_writes[0].dstBinding		 = 0;
					descriptor_writes[0].dstArrayElement = 0;
					descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptor_writes[0].descriptorCount = 1;
					descriptor_writes[0].pBufferInfo	 = &uniform_buffer_info;

					/* if (diffuse_image_buffer_infos.empty())
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
 */
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

	void MeshRenderer::Render(VkCommandBuffer command_buffer, uint32_t current_frame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		Vulkan::Utils::VulkanUniformBufferTransfer(
			renderer,
			this->pipeline,
			current_frame,
			&this->mat_mvp,
			sizeof(glm::mat4)
		);

		this->pipeline->BindPipeline(command_buffer, current_frame);

		VkBuffer vertex_buffers[] = {this->vbo->GetNativeHandle()};
		VkDeviceSize offsets[]	  = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(command_buffer, static_cast<uint32_t>(this->vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
