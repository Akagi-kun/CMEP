#include <assert.h>
#include <cstring>

#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Object.hpp"

#include "Engine.hpp"

namespace Engine::Rendering
{
	SpriteRenderer::SpriteRenderer()
	{
		// static const char* vertex_shader_source =
		// 	"#version 400 core\n"
		// 	"layout(location = 0) in vec3 pos; layout(location = 1) in vec2 texCord; out vec2 fragTexCord;"
		// 	"void main() { gl_Position = vec4(pos, 1.0f); fragTexCord = texCord; }";

		// static const char* fragment_shader_source =
		// 	"#version 400 core\n"
		// 	"out vec4 color; in vec2 fragTexCord; uniform sampler2D texture0;"
		// 	"void main() { color = texture(texture0, fragTexCord); }";

		// this->program = std::make_unique<Rendering::Shader>(vertex_shader_source, fragment_shader_source);
		
		VulkanRenderingEngine* renderer = global_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings = renderer->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		pipeline_settings.descriptorLayoutSettings.binding.push_back(1);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);

		this->pipeline = renderer->createVulkanPipeline(pipeline_settings, "game/shaders/vulkan/textrenderer_vert.spv", "game/shaders/vulkan/textrenderer_frag.spv");

	}

	SpriteRenderer::~SpriteRenderer()
	{
		//glDeleteVertexArrays(1, &this->vao);
		//glDeleteBuffers(1, &this->vbo);
	}

	void SpriteRenderer::Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny, glm::vec3 parent_position, glm::vec3 parent_rotation, glm::vec3 parent_size) noexcept
	{
		this->_pos = pos;
		this->_size = size;
		this->_rotation = rotation;

		this->_parent_pos = parent_position;
		this->_parent_rotation = parent_rotation;
		this->_parent_size = parent_size;

		this->_screenx = screenx;
		this->_screeny = screeny;

		this->has_updated_mesh = false;
	}

	void SpriteRenderer::UpdateTexture(const Rendering::Texture* texture) noexcept
	{
		this->texture.reset(texture);

		this->has_updated_mesh = false;
	}

	void SpriteRenderer::UpdateMesh() noexcept
	{
		this->has_updated_mesh = true;

		VulkanRenderingEngine* renderer = global_engine->GetRenderingEngine();

		if (this->vbo != nullptr)
		{
			vkDeviceWaitIdle(renderer->GetLogicalDevice());
			renderer->cleanupVulkanBuffer(this->vbo);
			this->vbo = nullptr;
		}
		
		const float xs = (float)this->_size.x * 2.0f;
		const float ys = (float)this->_size.y * 2.0f;
		const float x = (float)this->_pos.x * 2.0f - 1.0f;
		const float y = (float)this->_pos.y * 2.0f - 1.0f;

		const std::vector<RenderingVertex> vertices = {
			{{x, y + ys, 0.0},      {0.0, 0.0, 0.0}, {0.0, 1.0}},
			{{x + xs, y + ys, 0.0}, {0.0, 0.0, 0.0}, {1.0, 1.0}},
			{{x + xs, y, 0.0},      {0.0, 0.0, 0.0}, {1.0, 0.0}},
			{{x, y, 0.0},           {0.0, 0.0, 0.0}, {0.0, 0.0}},
		};

		this->vbo = renderer->createVulkanVertexBufferFromData(vertices);
		
		VulkanTextureImage* textureImage = this->texture->GetTextureImage();
		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = pipeline->uniformBuffers[i]->buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::mat4);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImage->image->imageView;
			imageInfo.sampler = textureImage->textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = pipeline->vkDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = pipeline->vkDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(renderer->GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
		//glNamedBufferData(this->vbo, sizeof(data), (void*)data, GL_STATIC_DRAW);
	}

	void SpriteRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->vkPipelineLayout, 0, 1, &this->pipeline->vkDescriptorSets[currentFrame], 0, nullptr);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
		VkBuffer vertexBuffers[] = { this->vbo->buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(commandBuffer, 4, 1, 0, 0);

		/*
		if (!(this->texture || this->program))
		{
			throw std::exception("No texture/program assigned to SpriteRenderer, cannot perform Engine::Rendering::SpriteRenderer::Render()");
		}

		GLuint texture = this->texture.get()->GetTexture();
		GLuint shader = this->program.get()->GetProgram();
		assert(texture != 0);
		assert(shader != 0);

		glBindVertexArray(this->vao);
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

		glUseProgram(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Vertex and texture coord data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)nullptr);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		*/
	}
}