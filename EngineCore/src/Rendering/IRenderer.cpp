#include "Rendering/IRenderer.hpp"

#include "Assets/Texture.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/Wrappers/VCommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/VSampledImage.hpp"

#include "vulkan/vulkan_core.h"

namespace Engine::Rendering
{
	void IRenderer::UpdateDescriptorSets()
	{
		this->has_updated_descriptors = true;

		if (this->texture)
		{
			Vulkan::VulkanRenderingEngine::per_frame_array<VkDescriptorImageInfo> descriptor_image_infos{};

			/* std::array<VkDescriptorImageInfo, Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight()>
				descriptor_image_infos{}; */
			Vulkan::VulkanRenderingEngine::per_frame_array<VkWriteDescriptorSet> descriptor_writes{};

			Vulkan::VSampledImage* texture_image = this->texture->GetTextureImage();

			for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_infos[i].imageView	  = texture_image->GetNativeViewHandle();
				descriptor_image_infos[i].sampler	  = texture_image->texture_sampler;

				descriptor_writes[i].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[i].dstSet			 = this->pipeline->GetDescriptorSet(i);
				descriptor_writes[i].dstBinding		 = 1;
				descriptor_writes[i].dstArrayElement = 0;
				descriptor_writes[i].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_writes[i].descriptorCount = 1;
				descriptor_writes[i].pImageInfo		 = &(descriptor_image_infos[i]);

				// TODO: Add support for additional textures?
			}

			this->pipeline->UpdateDescriptorSets(descriptor_writes);
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
			this->pipeline->GetUniformBuffer(current_frame)->MemoryCopy(&this->matrix_data, sizeof(RendererMatrixData));

			this->pipeline->BindPipeline(command_buffer->GetNativeHandle(), current_frame);

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

} // namespace Engine::Rendering
