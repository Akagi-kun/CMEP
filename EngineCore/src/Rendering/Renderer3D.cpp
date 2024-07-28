#include "Rendering/Renderer3D.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/framework.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"

#include <cassert>
#include <glm/gtc/quaternion.hpp>

namespace Engine::Rendering
{
	Renderer3D::Renderer3D(Engine* engine, IMeshBuilder* with_builder, const char* with_pipeline_program)
		: IRenderer(engine, with_builder, with_pipeline_program)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = this->mesh_builder->GetSupportedTopology();

		pipeline_settings.shader = this->pipeline_name;

		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			0,
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT,
		});

		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		this->pipeline =
			new Vulkan::VPipeline(renderer->GetDeviceManager(), pipeline_settings, renderer->GetRenderPass());
	}

	Renderer3D::~Renderer3D()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up Renderer3D");

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
			case RendererSupplyDataType::TEXTURE:
			{
				this->texture		   = std::static_pointer_cast<Texture>(data.payload_ptr);
				this->has_updated_mesh = false;
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

	void Renderer3D::UpdateMesh()
	{
		// TODO: Fix this (should be true, or just check another way)
		this->has_updated_mesh = false;

		auto* renderer		 = this->owner_engine->GetRenderingEngine();
		auto* device_manager = renderer->GetDeviceManager();

		Vulkan::VSampledImage* texture_image = nullptr;

		if (this->texture)
		{
			texture_image = this->texture->GetTextureImage();
		}

		for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
		{
			std::vector<VkWriteDescriptorSet> descriptor_writes{};

			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = this->pipeline->GetUniformBuffer(i)->GetNativeHandle();
			buffer_info.offset = 0;
			buffer_info.range  = sizeof(RendererMatrixData);

			VkWriteDescriptorSet uniform_buffer_set = {};
			uniform_buffer_set.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uniform_buffer_set.dstSet				= this->pipeline->GetDescriptorSet(i);
			uniform_buffer_set.dstBinding			= 0;
			uniform_buffer_set.dstArrayElement		= 0;
			uniform_buffer_set.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniform_buffer_set.descriptorCount		= 1;
			uniform_buffer_set.pBufferInfo			= &buffer_info;
			descriptor_writes.push_back(uniform_buffer_set);

			if (this->texture)
			{
				VkDescriptorImageInfo image_info{};
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView   = texture_image->GetNativeViewHandle();
				image_info.sampler	   = texture_image->texture_sampler;

				VkWriteDescriptorSet optional_texture_set = {};
				optional_texture_set.sType				  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				optional_texture_set.dstSet				  = pipeline->GetDescriptorSet(i);
				optional_texture_set.dstBinding			  = 1;
				optional_texture_set.dstArrayElement	  = 0;
				optional_texture_set.descriptorType		  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				optional_texture_set.descriptorCount	  = 1;
				optional_texture_set.pImageInfo			  = &image_info;
				descriptor_writes.push_back(optional_texture_set);
			}

			vkUpdateDescriptorSets(
				device_manager->GetLogicalDevice(),
				static_cast<uint32_t>(descriptor_writes.size()),
				descriptor_writes.data(),
				0,
				nullptr
			);
		}
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
