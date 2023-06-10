#include <assert.h>

#include "Rendering/GLCommon.hpp"

#include "Rendering/AxisRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Object.hpp"
#include "GlobalSceneManager.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace Engine::Rendering
{
	AxisRenderer::AxisRenderer()
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

	AxisRenderer::~AxisRenderer()
	{
		glDeleteVertexArrays(1, &this->vao);
		glDeleteBuffers(1, &this->vbo);
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

		if (this->vbo == 0)
		{
			glCreateBuffers(1, &this->vbo);
		}
		if (this->vao == 0)
		{
			glCreateVertexArrays(1, &this->vao);
		}

		const float xs = (float)this->_size.x * 2; const float ys = (float)this->_size.y * 2;
		const float x = (float)this->_pos.x * 2 - 1.f; const float y = (float)this->_pos.y * 2 - 1.f;
		const GLfloat data[] = {
			0, 0, 0, 1,
			0, 1, 0, 1,
			0, 0, 0, 2,
			1, 0, 0, 2,
			0, 0, 0, 3,
			0, 0, 0, 3
		};

		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)this->_screenx / this->_screeny, 0.1f, 100.0f);
		glm::mat4 View = global_scene_manager->GetCameraViewMatrix();

		//glm::mat4 Model = ;

		//this->matMVP = Projection * View * Model;

		glNamedBufferData(this->vbo, sizeof(data), (void*)data, GL_STATIC_DRAW);
	}

	void AxisRenderer::Render()
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		GLuint shader = this->program.get()->GetProgram();
		assert(texture != 0);
		assert(shader != 0);

		glBindVertexArray(this->vao);
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

		glUseProgram(shader);

		// Vertex and texture coord data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)nullptr);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_LINE, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

	}
}