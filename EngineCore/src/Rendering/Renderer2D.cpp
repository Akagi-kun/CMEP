#include "Rendering/Renderer2D.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/VPipeline.hpp"
#include "Rendering/framework.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Engine::Rendering
{
	Renderer2D::Renderer2D(Engine* engine, IMeshBuilder* with_builder, std::string_view with_pipeline_program)
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

		auto* device_manager = renderer->GetDeviceManager();

		// Update descriptor set for matrices
		for (uint32_t i = 0; i < Vulkan::VulkanRenderingEngine::GetMaxFramesInFlight(); i++)
		{
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

			vkUpdateDescriptorSets(device_manager->GetLogicalDevice(), 1, &uniform_buffer_set, 0, nullptr);
		}
	}

	Renderer2D::~Renderer2D()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up Renderer2D");

		delete this->pipeline;
	}

	void Renderer2D::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				auto font_cast				  = std::static_pointer_cast<Font>(data.payload_ptr);
				this->texture				  = font_cast->GetPageTexture(0);
				this->has_updated_descriptors = false;
				break;
			}
			case RendererSupplyDataType::TEXTURE:
			{
				this->texture				  = std::static_pointer_cast<Texture>(data.payload_ptr);
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
} // namespace Engine::Rendering
