#include "Rendering/Renderers/Renderer.hpp"

#include "Assets/Asset.hpp"
#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Vulkan/backend.hpp"
#include "Rendering/Vulkan/common.hpp"
#include "Rendering/Vulkan/rendering.hpp"
#include "Rendering/framework.hpp"

#include "Engine.hpp"
#include "InternalEngineObject.hpp"
#include "objects/CommandBuffer.hpp"
#include "vulkan/vulkan.hpp"

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

namespace Engine::Rendering
{
	IRenderer::IRenderer(
		Engine*			 with_engine,
		IMeshBuilder*	 with_builder,
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
			case RendererSupplyData::Type::FONT:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				auto font_cast			= std::static_pointer_cast<Font>(payload_ref.lock());
				texture					= font_cast->GetPageTexture(0);
				has_updated_descriptors = false;

				// TODO: Remove
				mesh_builder->SupplyData({MeshBuilderSupplyData::Type::FONT, font_cast});
				break;
			}
			case RendererSupplyData::Type::TEXTURE:
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

			vk::DescriptorImageInfo descriptor_image_info(
				*texture_image->texture_sampler,
				*texture_image->GetNativeViewHandle(),
				vk::ImageLayout::eShaderReadOnlyOptimal
			);

			vk::WriteDescriptorSet descriptor_write(
				{},
				1,
				0,
				1,
				vk::DescriptorType::eCombinedImageSampler,
				&descriptor_image_info,
				{},
				{}
			);

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
		matrix_data.mat_vp	  = projection;

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
