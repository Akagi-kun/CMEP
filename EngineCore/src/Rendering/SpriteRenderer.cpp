#include <assert.h>
#include <cstring>

#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Object.hpp"

namespace Engine::Rendering
{
	SpriteRenderer::SpriteRenderer()
	{
		static const char* vertex_shader_source =
			"#version 400 core\n"
			"layout(location = 0) in vec3 pos; layout(location = 1) in vec2 texCord; out vec2 fragTexCord;"
			"void main() { gl_Position = vec4(pos, 1.0f); fragTexCord = texCord; }";

		static const char* fragment_shader_source =
			"#version 400 core\n"
			"out vec4 color; in vec2 fragTexCord; uniform sampler2D texture0;"
			"void main() { color = texture(texture0, fragTexCord); }";

		this->program = std::make_unique<Rendering::Shader>(vertex_shader_source, fragment_shader_source);
	}

	SpriteRenderer::~SpriteRenderer()
	{
		//glDeleteVertexArrays(1, &this->vao);
		//glDeleteBuffers(1, &this->vbo);
	}

	void SpriteRenderer::Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept
	{
		this->_pos = pos;
		this->_size = size;
		this->_rotation = rotation;

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

		if (this->vbo == 0)
		{
			//glCreateBuffers(1, &this->vbo);
		}
		if (this->vao == 0)
		{
			//glCreateVertexArrays(1, &this->vao);
		}
		
		const float xs = (float)this->_size.x * 2; const float ys = (float)this->_size.y * 2;
		const float x = (float)this->_pos.x * 2 - 1.f; const float y = (float)this->_pos.y * 2 - 1.f;
		const float data[] = {
			x, y, 0.f,
			0.f, 1.f,
			xs + x, y, 0.f,
			1.f, 1.f,
			xs + x, ys + y, 0.f,
			1.f, 0.f,
			x, ys + y, 0.f,
			0.f, 0.f
		};

		//glNamedBufferData(this->vbo, sizeof(data), (void*)data, GL_STATIC_DRAW);
	}

	void SpriteRenderer::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

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