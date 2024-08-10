#include "Rendering/Renderers/Renderer.hpp"

#include "Assets/Texture.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Vulkan/PipelineManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/Wrappers/VCommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/VPipeline.hpp"
#include "Rendering/Vulkan/Wrappers/VSampledImage.hpp"
#include "Rendering/framework.hpp"

#include "Engine.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering
{
	IRenderer::IRenderer(
		Engine* with_engine,
		IMeshBuilder* with_builder,
		// Vulkan::PipelineManager* with_pipeline_manager,
		std::string_view with_pipeline_program
	)
		: InternalEngineObject(with_engine), pipeline_name(with_pipeline_program),
		  pipeline_manager(with_engine->GetVulkanPipelineManager()), mesh_builder(with_builder)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		this->settings =
			{renderer->GetSwapchainExtent(), this->pipeline_name, this->mesh_builder->GetSupportedTopology()};

		this->settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		// pipeline_manager->GetPipeline(pipeline_settings);
		//  this->pipeline =	new Vulkan::VPipeline(renderer->GetDeviceManager(), pipeline_settings,
		//  renderer->GetRenderPass());
	}

	void IRenderer::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				auto font_cast				  = std::static_pointer_cast<Font>(payload_ref.lock());
				this->texture				  = font_cast->GetPageTexture(0);
				this->has_updated_descriptors = false;
				break;
			}
			case RendererSupplyDataType::TEXTURE:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				this->texture				  = std::static_pointer_cast<Texture>(payload_ref.lock());
				this->has_updated_descriptors = false;
				break;
			}
			default:
			{
				break;
			}
		}

		assert(this->mesh_builder != nullptr && "This renderer has not been assigned a mesh builder!");
		this->mesh_builder->SupplyData(data);
	}

	void IRenderer::UpdateDescriptorSets()
	{
		this->has_updated_descriptors = true;

		auto extended_settings = Vulkan::ExtendedPipelineSettings{settings};
		if (texture)
		{
			extended_settings.supply_data.emplace_back(RendererSupplyDataType::TEXTURE, this->texture);
		}
		auto pipeline_result	  = this->pipeline_manager->GetPipeline(extended_settings);
		this->pipeline_user_index = std::get<size_t>(pipeline_result);
		this->pipeline			  = std::get<Vulkan::VPipeline*>(pipeline_result);

		if (this->texture)
		{
			Vulkan::VulkanRenderingEngine::per_frame_array<VkDescriptorImageInfo> descriptor_image_infos{};
			Vulkan::VulkanRenderingEngine::per_frame_array<VkWriteDescriptorSet> descriptor_writes{};

			Vulkan::VSampledImage* texture_image = this->texture->GetTextureImage();

			for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_infos[i].imageView	  = texture_image->GetNativeViewHandle();
				descriptor_image_infos[i].sampler	  = texture_image->texture_sampler;

				descriptor_writes[i].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[i].dstSet			 = this->pipeline->GetDescriptorSet(0, i);
				descriptor_writes[i].dstBinding		 = 1;
				descriptor_writes[i].dstArrayElement = 0;
				descriptor_writes[i].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_writes[i].descriptorCount = 1;
				descriptor_writes[i].pImageInfo		 = &(descriptor_image_infos[i]);

				// TODO: Add support for additional textures?
			}

			// TODO: Update using single VkWriteDescriptorSet?
			this->pipeline->UpdateDescriptorSets(this->pipeline_user_index, descriptor_writes);
		}
	}

	void IRenderer::Render(Vulkan::VCommandBuffer* command_buffer, uint32_t current_frame)
	{
		if (!this->has_updated_matrices)
		{
			this->UpdateMatrices();
		}

		if (!this->has_updated_descriptors)
		{
			this->UpdateDescriptorSets();
		}

		// If builder requests a rebuild, do it and update current context
		if (this->mesh_builder->NeedsRebuild())
		{
			this->mesh_builder->Build();
		}
		this->mesh_context = this->mesh_builder->GetContext();

		// Render only if VBO non-empty
		if (this->mesh_context.vbo_vert_count > 0)
		{
			this->pipeline->GetUniformBuffer(this->pipeline_user_index, current_frame)
				->MemoryCopy(&this->matrix_data, sizeof(RendererMatrixData));

			this->pipeline->BindPipeline(this->pipeline_user_index, command_buffer->GetNativeHandle(), current_frame);

			VkBuffer vertex_buffers[] = {this->mesh_context.vbo->GetNativeHandle()};
			VkDeviceSize offsets[]	  = {0};
			vkCmdBindVertexBuffers(command_buffer->GetNativeHandle(), 0, 1, vertex_buffers, offsets);

			vkCmdDraw(
				command_buffer->GetNativeHandle(),
				static_cast<uint32_t>(this->mesh_context.vbo_vert_count),
				1,
				0,
				0
			);
		}
	}

	void Renderer2D::UpdateMatrices()
	{
		glm::mat4 projection{};
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			projection = locked_scene_manager->GetProjectionMatrixOrtho();
		}

		this->matrix_data.mat_model = CalculateModelMatrix(this->transform, this->parent_transform);
		this->matrix_data.mat_vp	= projection; // * view;

		this->has_updated_matrices = true;
	}

	void Renderer3D::UpdateMatrices()
	{
		glm::mat4 view;
		glm::mat4 projection;
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			view	   = locked_scene_manager->GetCameraViewMatrix();
			projection = locked_scene_manager->GetProjectionMatrix(this->screen);
		}
		projection[1][1] *= -1;

		this->matrix_data.mat_model = CalculateModelMatrix(this->transform, this->parent_transform);
		this->matrix_data.mat_vp	= projection * view;

		this->has_updated_matrices = true;
	}

} // namespace Engine::Rendering
