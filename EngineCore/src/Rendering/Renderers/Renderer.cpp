#include "Rendering/Renderers/Renderer.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Vulkan/backend.hpp"
#include "Rendering/Vulkan/exports.hpp"
#include "Rendering/framework.hpp"

#include "Engine.hpp"
#include "vulkan/vulkan.hpp"

namespace Engine::Rendering
{
	IRenderer::IRenderer(
		Engine* with_engine,
		IMeshBuilder* with_builder,
		std::string_view with_pipeline_program
	)
		: InternalEngineObject(with_engine), pipeline_name(with_pipeline_program),
		  pipeline_manager(with_engine->GetVulkanPipelineManager()), mesh_builder(with_builder)
	{
		Vulkan::Instance* instance = this->owner_engine->GetVulkanInstance();

		settings = {
			instance->GetWindow()->GetSwapchain()->GetExtent(),
			pipeline_name,
			mesh_builder->GetSupportedTopology()
		};

		settings.descriptor_settings.emplace(
			1,
			Vulkan::DescriptorBindingSetting{
				1,
				vk::DescriptorType::eCombinedImageSampler,
				vk::ShaderStageFlagBits::eFragment,
				{}
			}
		);
	}

	IRenderer::~IRenderer()
	{
		delete pipeline;

		delete mesh_builder;
		mesh_builder = nullptr;
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

		if (texture)
		{
			settings.descriptor_settings[1].opt_match_hash = std::hash<Asset>{}(*texture);
		}

		pipeline = pipeline_manager->GetPipeline(settings);

		if (texture)
		{
			auto* texture_image = texture->GetTextureImage();

			VkDescriptorImageInfo descriptor_image_info{};
			descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptor_image_info.imageView	  = *texture_image->GetNativeViewHandle();
			descriptor_image_info.sampler	  = *texture_image->texture_sampler;

			VkWriteDescriptorSet descriptor_write{};
			descriptor_write.sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write.dstBinding		 = 1;
			descriptor_write.dstArrayElement = 0;
			descriptor_write.descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_write.descriptorCount = 1;
			descriptor_write.pImageInfo		 = &(descriptor_image_info);

			pipeline->UpdateDescriptorSetsAll(descriptor_write);
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
			pipeline->GetUniformBuffer(current_frame)
				->MemoryCopy(&matrix_data, sizeof(RendererMatrixData));

			pipeline->BindPipeline(*command_buffer->GetHandle(), current_frame);

			command_buffer->GetHandle().bindVertexBuffers(0, {*mesh_context.vbo->GetHandle()}, {0});

			command_buffer->GetHandle()
				.draw(static_cast<uint32_t>(mesh_context.vbo_vert_count), 1, 0, 0);
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
