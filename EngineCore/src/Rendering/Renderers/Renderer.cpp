#include "Rendering/Renderers/Renderer.hpp"

#include "Assets/Texture.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Vulkan/exports.hpp"
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
		Vulkan::Instance* instance = this->owner_engine->GetVulkanInstance();

		settings =
			{instance->GetWindow()->GetSwapchain()->GetExtent(), pipeline_name, mesh_builder->GetSupportedTopology()};

		settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		// pipeline_manager->GetPipeline(pipeline_settings);
		//  pipeline =	new Vulkan::VPipeline(renderer->GetDeviceManager(), pipeline_settings,
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

				auto font_cast			= std::static_pointer_cast<Font>(payload_ref.lock());
				texture					= font_cast->GetPageTexture(0);
				has_updated_descriptors = false;
				break;
			}
			case RendererSupplyDataType::TEXTURE:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				texture					= std::static_pointer_cast<Texture>(payload_ref.lock());
				has_updated_descriptors = false;
				break;
			}
			default:
			{
				break;
			}
		}

		assert(mesh_builder != nullptr && "This renderer has not been assigned a mesh builder!");
		mesh_builder->SupplyData(data);
	}

	void IRenderer::UpdateDescriptorSets()
	{
		has_updated_descriptors = true;

		auto extended_settings = Vulkan::ExtendedPipelineSettings{settings};
		if (texture)
		{
			extended_settings.supply_data.emplace_back(RendererSupplyDataType::TEXTURE, texture);
		}
		auto pipeline_result = pipeline_manager->GetPipeline(extended_settings);
		pipeline_user_index	 = std::get<size_t>(pipeline_result);
		pipeline			 = std::get<Vulkan::Pipeline*>(pipeline_result);

		if (texture)
		{
			Vulkan::SampledImage* texture_image = texture->GetTextureImage();

			VkDescriptorImageInfo descriptor_image_info{};
			descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptor_image_info.imageView	  = texture_image->GetNativeViewHandle();
			descriptor_image_info.sampler	  = texture_image->texture_sampler;

			VkWriteDescriptorSet descriptor_write{};
			descriptor_write.sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write.dstBinding		 = 1;
			descriptor_write.dstArrayElement = 0;
			descriptor_write.descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_write.descriptorCount = 1;
			descriptor_write.pImageInfo		 = &(descriptor_image_info);

			pipeline->UpdateDescriptorSetsAll(pipeline_user_index, descriptor_write);
		}
	}

	void IRenderer::Render(Vulkan::CommandBuffer* command_buffer, uint32_t current_frame)
	{
		if (!has_updated_matrices)
		{
			UpdateMatrices();
		}

		if (!has_updated_descriptors)
		{
			UpdateDescriptorSets();
		}

		// If builder requests a rebuild, do it
		if (mesh_builder->NeedsRebuild())
		{
			mesh_builder->Build();
		}
		mesh_context = mesh_builder->GetContext();

		// Render only if VBO non-empty
		if (mesh_context.vbo_vert_count > 0)
		{
			pipeline->GetUniformBuffer(pipeline_user_index, current_frame)
				->MemoryCopy(&matrix_data, sizeof(RendererMatrixData));

			pipeline->BindPipeline(pipeline_user_index, *command_buffer, current_frame);

			VkBuffer vertex_buffers[] = {*mesh_context.vbo};
			VkDeviceSize offsets[]	  = {0};
			vkCmdBindVertexBuffers(*command_buffer, 0, 1, vertex_buffers, offsets);

			vkCmdDraw(*command_buffer, static_cast<uint32_t>(mesh_context.vbo_vert_count), 1, 0, 0);
		}
	}

	void Renderer2D::UpdateMatrices()
	{
		glm::mat4 projection{};
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			projection = locked_scene_manager->GetProjectionMatrixOrtho();
		}

		matrix_data.mat_model = CalculateModelMatrix(transform, parent_transform);
		matrix_data.mat_vp	  = projection; // * view;

		has_updated_matrices = true;
	}

	void Renderer3D::UpdateMatrices()
	{
		glm::mat4 view;
		glm::mat4 projection;
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			view	   = locked_scene_manager->GetCameraViewMatrix();
			projection = locked_scene_manager->GetProjectionMatrix(screen);
		}
		projection[1][1] *= -1;

		matrix_data.mat_model = CalculateModelMatrix(transform, parent_transform);
		matrix_data.mat_vp	  = projection * view;

		has_updated_matrices = true;
	}

} // namespace Engine::Rendering
