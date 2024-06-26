#include "Rendering/TextRenderer.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Engine::Rendering
{
	TextRenderer::TextRenderer(Engine* engine) : IRenderer(engine, nullptr)
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings  = renderer->GetVulkanDefaultPipelineSettings();
		pipeline_settings.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.descriptor_layout_settings.push_back(
			VulkanDescriptorLayoutSettings{0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
		);
		// pipeline_settings.descriptor_layout_settings.binding.push_back(0);
		// pipeline_settings.descriptor_layout_settings.descriptor_count.push_back(1);
		// pipeline_settings.descriptor_layout_settings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		// pipeline_settings.descriptor_layout_settings.stage_flags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		pipeline_settings.descriptor_layout_settings.push_back(VulkanDescriptorLayoutSettings{
			1,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});
		// pipeline_settings.descriptor_layout_settings.binding.push_back(1);
		// pipeline_settings.descriptor_layout_settings.descriptor_count.push_back(1);
		// pipeline_settings.descriptor_layout_settings.types.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		// pipeline_settings.descriptor_layout_settings.stage_flags.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);

		this->pipeline = renderer->CreateVulkanPipeline(
			pipeline_settings,
			"game/shaders/vulkan/textrenderer_vert.spv",
			"game/shaders/vulkan/textrenderer_frag.spv"
		);
	}

	TextRenderer::~TextRenderer()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up text renderer");
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		vkDeviceWaitIdle(renderer->GetLogicalDevice());

		if (this->vbo != nullptr)
		{
			renderer->CleanupVulkanBuffer(this->vbo);
		}
		renderer->CleanupVulkanPipeline(this->pipeline);
	}

	void TextRenderer::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				this->font			   = std::static_pointer_cast<Font>(data.payload_ptr);
				this->has_updated_mesh = false;
				return;
			}
			case RendererSupplyDataType::TEXT:
			{
				this->text.assign(data.payload_string);
				this->has_updated_mesh = false;
				return;
			}
			default:
			{
				break;
			}
		}

		throw std::runtime_error("Tried to supply Renderer data with payload type unsupported by the renderer!");
	}

	void TextRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (this->vbo != nullptr)
		{
			vkDeviceWaitIdle(renderer->GetLogicalDevice());
			renderer->CleanupVulkanBuffer(this->vbo);
			this->vbo = nullptr;
		}

		int font_size = std::stoi(*this->font->GetFontInfoParameter("size"));
		// int font_line_height = std::stoi(*this->font->GetFontInfoParameter("lineHeight"));
		assert(
			font_size > 0 && ".fnt file should have a defined 'size' parameter but size acquired was 0 or less! (is "
							 "font correctly loaded?)"
		);
		// assert(
		//	font_line_height > 0 && ".fnt file should have a defined 'lineHeight' parameter but lineHeight acquired "
		//							"was 0 or less! (is font correctly loaded?)"
		//);

		std::vector<RenderingVertex> generated_mesh = {};

		// Positions that denote the origin for a specific character
		// Moved as the mesh is built
		float char_origin_x = 0.f;
		float char_origin_y = 0.f;
		for (size_t i = 0; i < this->text.size(); i++)
		{
			this->vbo = nullptr;

			// Following calculations assume that font sizes are specified in pixels
			// transform.size.y is rounded
			//
			// Ratio between font size (as defined by the .fnt file) and selected size for this renderer
			// value 0.0 denotes 0 pixels and 1.0 denotes font_size (native) pixels
			// used as a size multiplier for the rendered mesh
			const auto font_size_ratio = static_cast<float>(
				std::round(this->transform.size.y) / static_cast<float>(font_size)
			);

			// Denotes how the line height translates into screen space coordinates
			const auto line_height_screenspace = static_cast<float>(
				std::round(this->transform.size.y) / static_cast<float>(this->screen.y)
			);

			if (this->text[i] == '\n')
			{
				// Simulate a newline
				char_origin_y += line_height_screenspace;
				char_origin_x = 0.f;
			}
			else
			{
				// Get character info
				Rendering::FontChar* character = this->font->GetChar(this->text[i]);
				assert(character != nullptr);
				if (character == nullptr)
				{
					this->logger
						->SimpleLog(Logging::LogLevel::Error, "Char 0x%x is not found in set font", this->text[i]);
					continue;
				}

				// Get texture
				uint_fast32_t texture_width		 = 0;
				uint_fast32_t texture_height	 = 0;
				std::shared_ptr<Texture> texture = this->font->GetPageTexture(character->page);
				assert(texture != nullptr);
				texture->GetSize(texture_width, texture_height);
				assert(texture_width > 0 && texture_height > 0);

				const auto char_x	   = static_cast<float>(character->x);
				const auto char_width  = static_cast<float>(character->width);
				const auto char_y	   = static_cast<float>(character->y);
				const auto char_height = static_cast<float>(character->height);

				// Obscure math I don't understand, achieved with trial and error and works so just leave it like this
				const float size_x = char_width / static_cast<float>(this->screen.x) *
									 static_cast<float>(
										 std::round(this->transform.size.x) / static_cast<float>(font_size)
									 );
				const float size_y = char_height / static_cast<float>(this->screen.y) *
									 static_cast<float>(
										 std::round(this->transform.size.y) / static_cast<float>(font_size)
									 );

				// Offset origin by xoffset of this char
				const float position_x = char_origin_x + ((static_cast<float>(character->xoffset) * font_size_ratio) /
														  static_cast<float>(this->screen.x));
				const float position_y = char_origin_y;
				const float position_z = 0.0f;

				const float color_r = 1.0f;
				const float color_g = 1.0f;
				const float color_b = 1.0f;

				// position_x is already set so we can
				// move origin to the next character using xadvance from font
				char_origin_x += (static_cast<float>(character->xadvance) * font_size_ratio) /
								 static_cast<float>(this->screen.x);

				// If this character is a space we can skip generating a mesh for it
				if (this->text[i] == ' ')
				{
					continue;
				}

				std::array<RenderingVertex, 6> vertices = {};
				vertices[0]								= {
					glm::vec3(position_x, position_y + size_y, position_z),
					glm::vec3(color_r, color_g, color_b),
					glm::vec2(
						(char_x) / static_cast<float>(texture_width),
						(char_y + char_height) / static_cast<float>(texture_height)
					)
				};
				vertices[1] = {
					glm::vec3(position_x + size_x, position_y + size_y, position_z),
					glm::vec3(color_r, color_g, color_b),
					glm::vec2(
						(char_x + char_width) / static_cast<float>(texture_width),
						(char_y + char_height) / static_cast<float>(texture_height)
					)
				};
				vertices[2] = {
					glm::vec3(position_x, position_y, position_z),
					glm::vec3(color_r, color_g, color_b),
					glm::vec2(
						(char_x) / static_cast<float>(texture_width),
						(char_y) / static_cast<float>(texture_height)
					)
				};
				vertices[3] = {
					glm::vec3(position_x + size_x, position_y + size_y, position_z),
					glm::vec3(color_r, color_g, color_b),
					glm::vec2(
						(char_x + char_width) / static_cast<float>(texture_width),
						(char_y + char_height) / static_cast<float>(texture_height)
					)
				};
				vertices[4] = {
					glm::vec3(position_x + size_x, position_y, position_z),
					glm::vec3(color_r, color_g, color_b),
					glm::vec2(
						(char_x + char_width) / static_cast<float>(texture_width),
						(char_y) / static_cast<float>(texture_height)
					)
				};
				vertices[5] = {
					glm::vec3(position_x, position_y, position_z),
					glm::vec3(color_r, color_g, color_b),
					glm::vec2(
						(char_x) / static_cast<float>(texture_width),
						(char_y) / static_cast<float>(texture_height)
					)
				};

				generated_mesh.insert(generated_mesh.end(), vertices.begin(), vertices.end());

				this->texture_image = texture->GetTextureImage();
			}
		}

		glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f); //, -10.0f, 10.0f);

		if (this->parent_transform.size.x == 0.0f && this->parent_transform.size.y == 0.0f &&
			this->parent_transform.size.z == 0.0f)
		{
			this->parent_transform.size = glm::vec3(1, 1, 1);
		}

		glm::quat model_rotation  = glm::quat(glm::radians(this->transform.rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(this->parent_transform.rotation));
		glm::mat4 model			  = glm::translate(
								glm::translate(glm::mat4(1.0f), this->parent_transform.pos) *
									glm::mat4_cast(parent_rotation),
								this->transform.pos
							) *
						  glm::mat4_cast(model_rotation);

		this->mat_mvp = projection * model;

		assert(generated_mesh.size() > 0);

		this->vbo_vert_count = generated_mesh.size();

		this->vbo = renderer->CreateVulkanVertexBufferFromData(generated_mesh);

		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = pipeline->uniform_buffers[i]->buffer;
			buffer_info.offset = 0;
			buffer_info.range  = sizeof(glm::mat4);

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView   = this->texture_image->image->image_view;
			image_info.sampler	   = this->texture_image->texture_sampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet			 = pipeline->vk_descriptor_sets[i];
			descriptor_writes[0].dstBinding		 = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo	 = &buffer_info;

			descriptor_writes[1].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet			 = pipeline->vk_descriptor_sets[i];
			descriptor_writes[1].dstBinding		 = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo		 = &image_info;

			vkUpdateDescriptorSets(
				renderer->GetLogicalDevice(),
				static_cast<uint32_t>(descriptor_writes.size()),
				descriptor_writes.data(),
				0,
				nullptr
			);
		}

		// renderer->updateVulkanDescriptorSetsVulkanTextureImage(this->pipeline, this->textureImage);
	}

	void TextRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (this->text.size() == 0)
		{
			return;
		}

		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();
		vkMapMemory(
			renderer->GetLogicalDevice(),
			pipeline->uniform_buffers[currentFrame]->allocation_info.deviceMemory,
			pipeline->uniform_buffers[currentFrame]->allocation_info.offset,
			pipeline->uniform_buffers[currentFrame]->allocation_info.size,
			0,
			&(pipeline->uniform_buffers[currentFrame]->mapped_data)
		);

		memcpy(this->pipeline->uniform_buffers[currentFrame]->mapped_data, &this->mat_mvp, sizeof(glm::mat4));

		vkUnmapMemory(
			renderer->GetLogicalDevice(),
			pipeline->uniform_buffers[currentFrame]->allocation_info.deviceMemory
		);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline->vk_pipeline_layout,
			0,
			1,
			&this->pipeline->vk_descriptor_sets[currentFrame],
			0,
			nullptr
		);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
		VkBuffer vertex_buffers[] = {this->vbo->buffer};
		VkDeviceSize offsets[]	  = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, offsets);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(this->vbo_vert_count), 1, 0, 0);
	}
} // namespace Engine::Rendering
