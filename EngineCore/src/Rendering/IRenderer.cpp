#include "Rendering/IRenderer.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/VCommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/VSampledImage.hpp"

#include "Engine.hpp"

namespace Engine::Rendering
{
	void IRenderer::UpdateDescriptorSets()
	{
		this->has_updated_descriptors = true;

		auto* renderer		 = this->owner_engine->GetRenderingEngine();
		auto* device_manager = renderer->GetDeviceManager();

		if (this->texture)
		{
			Vulkan::VSampledImage* texture_image = this->texture->GetTextureImage();

			for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				std::array<VkWriteDescriptorSet, 1> descriptor_writes{};

				VkDescriptorImageInfo image_info{};
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView   = texture_image->GetNativeViewHandle();
				image_info.sampler	   = texture_image->texture_sampler;

				VkWriteDescriptorSet& texture_set0 = descriptor_writes[0];
				texture_set0.sType				   = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				texture_set0.dstSet				   = pipeline->GetDescriptorSet(i);
				texture_set0.dstBinding			   = 1;
				texture_set0.dstArrayElement	   = 0;
				texture_set0.descriptorType		   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				texture_set0.descriptorCount	   = 1;
				texture_set0.pImageInfo			   = &image_info;

				// TODO: Add support for additional textures?

				if (!descriptor_writes.empty())
				{
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
