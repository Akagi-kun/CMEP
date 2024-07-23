#include "Rendering/Renderer2D.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/VPipeline.hpp"
#include "Rendering/framework.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <array>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Engine::Rendering
{
	Renderer2D::Renderer2D(Engine* engine, IMeshBuilder* with_builder, const char* with_pipeline_program)
		: IRenderer(engine, with_builder, with_pipeline_program)
	{
		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = this->mesh_builder->GetSupportedTopology();

		pipeline_settings.shader = this->pipeline_name;

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);
		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});

		this->pipeline =
			new Vulkan::VPipeline(renderer->GetDeviceManager(), pipeline_settings, renderer->GetRenderPass());
	}
	Renderer2D::~Renderer2D()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up text renderer");

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		renderer->SyncDeviceWaitIdle();

		delete this->pipeline;
	}

	void Renderer2D::SupplyData(const RendererSupplyData& data)
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

	void Renderer2D::UpdateMesh()
	{
		this->has_updated_mesh = true;

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->mesh_builder != nullptr)
		{
			this->mesh_builder->Build();
		}

		glm::mat4 projection{};
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			projection = locked_scene_manager->GetProjectionMatrixOrtho();
		}

		if (this->parent_transform.size.x == 0.0f || this->parent_transform.size.y == 0.0f ||
			this->parent_transform.size.z == 0.0f)
		{
			this->parent_transform.size = glm::vec3(1, 1, 1);
		}

		this->matrix_data.mat_model = CalculateModelMatrix(this->transform, this->parent_transform);
		this->matrix_data.mat_vp	= projection; // * view

		auto* texture_image = this->texture->GetTextureImage();

		if (auto* device_manager = renderer->GetDeviceManager())
		{
			for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
			{
				VkDescriptorBufferInfo buffer_info{};
				buffer_info.buffer = pipeline->GetUniformBuffer(i)->GetNativeHandle();
				buffer_info.offset = 0;
				buffer_info.range  = sizeof(RendererMatrixData);

				VkDescriptorImageInfo image_info{};
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView   = texture_image->GetNativeViewHandle();
				image_info.sampler	   = texture_image->texture_sampler;

				std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

				descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[0].dstSet			 = pipeline->GetDescriptorSet(i);
				descriptor_writes[0].dstBinding		 = 0;
				descriptor_writes[0].dstArrayElement = 0;
				descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptor_writes[0].descriptorCount = 1;
				descriptor_writes[0].pBufferInfo	 = &buffer_info;

				descriptor_writes[1].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_writes[1].dstSet			 = pipeline->GetDescriptorSet(i);
				descriptor_writes[1].dstBinding		 = 1;
				descriptor_writes[1].dstArrayElement = 0;
				descriptor_writes[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_writes[1].descriptorCount = 1;
				descriptor_writes[1].pImageInfo		 = &image_info;

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
} // namespace Engine::Rendering
