#include "Rendering/Renderers/Renderer.hpp"

#include "Assets/Asset.hpp"
#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/backend.hpp"
#include "Rendering/Vulkan/common.hpp"
#include "Rendering/Vulkan/rendering.hpp"

#include "Engine.hpp"
#include "InternalEngineObject.hpp"
#include "objects/CommandBuffer.hpp"
#include "vulkan/vulkan.hpp"

#include "glm/gtc/quaternion.hpp"

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
		  pipeline_manager(with_engine->getVulkanPipelineManager()), mesh_builder(with_builder)
	{
		Vulkan::Instance* instance = owner_engine->getVulkanInstance();

		settings = {
			instance->getWindow()->getSwapchain()->getExtent(),
			pipeline_name,
			mesh_builder->getSupportedTopology()
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
	}

	void IRenderer::supplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyData::Type::FONT:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				auto font_cast			= std::static_pointer_cast<Font>(payload_ref.lock());
				/**
				 * @todo Support multiple-page fonts?
				 */
				//       page 0 may not be guaranteed to be present
				texture					= font_cast->getPageTexture(0);
				has_updated_descriptors = false;

				/**
				 * @todo Remove
				 */
				mesh_builder->supplyData({MeshBuilderSupplyData::Type::FONT, font_cast});
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

	void IRenderer::updateDescriptorSets()
	{
		has_updated_descriptors = true;

		if (texture)
		{
			settings.descriptor_settings[1].opt_match_hash = std::hash<Asset>{}(*texture);
		}

		pipeline = pipeline_manager->getPipeline(settings);

		if (texture)
		{
			auto* image	  = texture->getImage();
			auto* sampler = texture->getSampler();

			vk::DescriptorImageInfo descriptor_image_info{
				.sampler	 = *sampler,
				.imageView	 = *image->getNativeViewHandle(),
				.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
			};

			vk::WriteDescriptorSet descriptor_write{
				.dstSet			 = {},
				.dstBinding		 = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType	 = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo		 = &descriptor_image_info
			};

			pipeline->updateDescriptorSetsAll(descriptor_write);
		}
	}

	void IRenderer::render(Vulkan::CommandBuffer* command_buffer, uint32_t current_frame)
	{
		if (!has_updated_matrices)
		{
			updateMatrices();
		}

		if (!has_updated_descriptors)
		{
			updateDescriptorSets();
		}

		// If builder requests a rebuild, do it
		if (mesh_builder->needsRebuild())
		{
			mesh_builder->build();
		}
		mesh_context = mesh_builder->getContext();

		// Render only if VBO non-empty
		if (mesh_context.vbo_vert_count > 0)
		{
			pipeline->getUniformBuffer(current_frame)
				->memoryCopy(&matrix_data, sizeof(RendererMatrixData));

			pipeline->bindPipeline(*command_buffer, current_frame);

			command_buffer->bindVertexBuffers(0, {*mesh_context.vbo->getHandle()}, {0});

			command_buffer->draw(static_cast<uint32_t>(mesh_context.vbo_vert_count), 1, 0, 0);
		}
	}

	namespace
	{
		/**
		 * Apply a single transform onto a matrix.
		 *
		 * @param[in] transform Transform to apply.
		 * @param[in] apply_to Matrix to apply to or an identity matrix if omitted.
		 * @return The resulting matrix.
		 */
		[[nodiscard]] glm::mat4 applyTransformToMatrix(
			const Transform& transform,
			const glm::mat4& apply_to = glm::identity<glm::mat4>()
		)
		{
			const glm::quat quat_rotation = glm::quat(glm::radians(transform.rotation));

			// model = position * rotation * scale
			auto translated = glm::translate(apply_to, transform.pos);
			auto rotated	= translated * glm::mat4_cast(quat_rotation);
			auto scaled		= glm::scale(rotated, transform.size);

			return scaled;
		}

		/**
		 * Calculate model matrix of an object.
		 *
		 * @param[in] local Transform of the current object.
		 * @param[in] parent Transform of the parent object.
		 * @return The resulting matrix.
		 */
		[[nodiscard]] glm::mat4 calculateModelMatrix(const Transform& local, const Transform& parent)
		{
			glm::mat4 model_parent = applyTransformToMatrix(parent);
			glm::mat4 model		   = applyTransformToMatrix(local, model_parent);

			return model;
		}
	} // namespace

	void Renderer2D::updateMatrices()
	{
		glm::mat4 projection{};

		auto scene_manager = owner_engine->getSceneManager().lock();
		assert(scene_manager);

		projection = scene_manager->getProjectionMatrixOrtho();

		matrix_data.mat_model = calculateModelMatrix(transform, parent_transform);
		matrix_data.mat_vp	  = projection;

		has_updated_matrices = true;
	}

	void Renderer3D::updateMatrices()
	{
		glm::mat4 view;
		glm::mat4 projection;

		auto scene_manager = owner_engine->getSceneManager().lock();
		assert(scene_manager);

		view	   = scene_manager->getCameraViewMatrix();
		projection = scene_manager->getProjectionMatrix();

		matrix_data.mat_model = calculateModelMatrix(transform, parent_transform);
		matrix_data.mat_vp	  = projection * view;

		has_updated_matrices = true;
	}

} // namespace Engine::Rendering
