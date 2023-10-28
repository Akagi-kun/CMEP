#include <assert.h>

#include <fstream>
#include <sstream>

#include "Rendering/AxisRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Object.hpp"
#include "GlobalSceneManager.hpp"
#include "Logging/Logging.hpp"

#include "Engine.hpp"

namespace Engine::Rendering
{
	AxisRenderer::AxisRenderer()
	{
		/*std::ifstream vert("data/shaders/axisrenderer.vert");
		std::ifstream frag("data/shaders/axisrenderer.frag");
		std::ostringstream vertsstr;
		std::ostringstream fragsstr;
		vertsstr << vert.rdbuf();
		fragsstr << frag.rdbuf();*/

		VulkanRenderingEngine* renderer = global_engine->GetRenderingEngine();

		VulkanPipelineSettings pipeline_settings = renderer->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		this->pipeline = renderer->createVulkanPipeline(pipeline_settings, "game/shaders/vulkan/axisrenderer_vert.spv", "game/shaders/vulkan/axisrenderer_frag.spv");

		//this->program = std::make_unique<Shader>(vertsstr.str().c_str(), fragsstr.str().c_str());
	}

	AxisRenderer::~AxisRenderer()
	{
		global_engine->GetRenderingEngine()->cleanupVulkanBuffer(this->vbo);
		global_engine->GetRenderingEngine()->cleanupVulkanPipeline(this->pipeline);
		//glDeleteVertexArrays(1, &this->vao);
		//glDeleteBuffers(1, &this->vbo);
	}

	void AxisRenderer::Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept
	{
		this->_pos = pos;
		this->_size = size;
		this->_rotation = rotation;

		this->_screenx = screenx;
		this->_screeny = screeny;

		this->has_updated_mesh = false;
	}

	void AxisRenderer::UpdateMesh() noexcept
	{
		this->has_updated_mesh = true;
	
		/*
		if (this->vbo == 0)
		{
			//glCreateBuffers(1, &this->vbo);
		}
		
		if (this->vao == 0)
		{
			//glCreateVertexArrays(1, &this->vao);
		}
		*/
		const GLfloat data_vert[] = {
			0.0, 0.0, 0.0,
			1.0, 0.0, 0.0,
			0.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 0.0, 0.0,
			0.0, 0.0, 1.0,
		};

		const GLint data_axis[] = {
			1, 1, 2, 2, 3, 3
		};

		const std::vector<RenderingVertex> vertices = {
			{{0.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}},
			{{1.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}},
			{{0.0, 0.0, 0.0}, {0.0f, 0.0f, 1.0f}},
			{{0.0, 1.0, 0.0}, {0.0f, 0.0f, 1.0f}},
			{{0.0, 0.0, 0.0}, {1.0f, 0.0f, 0.0f}},
			{{0.0, 0.0, 1.0}, {1.0f, 0.0f, 0.0f}}
		};

		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)this->_screenx / this->_screeny, 0.1f, 100.0f);
		glm::mat4 View = global_scene_manager->GetCameraViewMatrix();
		glm::mat4 Model = glm::mat4(1.0f);

		Projection[1][1] *= -1;

		this->matMVP = Projection * View * Model;

		VulkanRenderingEngine* renderer = global_engine->GetRenderingEngine();

		if (this->vbo != nullptr)
		{
			vkDeviceWaitIdle(renderer->GetLogicalDevice());
			renderer->cleanupVulkanBuffer(this->vbo);
			this->vbo = nullptr;
		}

		this->vbo = global_engine->GetRenderingEngine()->createVulkanVertexBufferFromData(vertices);

		for (size_t i = 0; i < renderer->GetMaxFramesInFlight(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = pipeline->uniformBuffers[i]->buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::mat4);

			std::vector<VkWriteDescriptorSet> descriptorWrites{};
			descriptorWrites.resize(1);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = pipeline->vkDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(renderer->GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		//glNamedBufferData(this->vbo, sizeof(data_vert) + sizeof(data_axis), NULL, GL_STATIC_DRAW);
		//glNamedBufferSubData(this->vbo, 0, sizeof(data_vert), (void*)&data_vert[0]);
		//glNamedBufferSubData(this->vbo, sizeof(data_vert), sizeof(data_axis), (void*)&data_axis[0]);
	}

	void AxisRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		memcpy(this->pipeline->uniformBuffers[currentFrame]->mappedMemory, &this->matMVP, sizeof(glm::mat4));
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->vkPipelineLayout, 0, 1, &this->pipeline->vkDescriptorSets[currentFrame], 0, nullptr);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
		VkBuffer vertexBuffers[] = { this->vbo->buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		
		//GLuint shader = this->program.get()->GetProgram();
		//assert(shader != 0);

		//glUseProgram(shader);

		//glBindVertexArray(this->vao);

		// Vertex and texture coord data
		//glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);
		//glVertexAttribIPointer(1, 1, GL_INT, 0, (void*)(18 * sizeof(GLfloat)));

		//GLuint mvp_uniform_id = glGetUniformLocation(shader, "matMVP");
		//glUniformMatrix4fv(mvp_uniform_id, 1, GL_FALSE, &this->matMVP[0][0]);

		//glEnableVertexAttribArray(0);
		//glEnableVertexAttribArray(1);

		//glLineWidth(5.0);
		//glDrawArrays(GL_LINES, 0, 6);

		//glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);
	}
}