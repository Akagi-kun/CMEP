#include <assert.h>

#include "Rendering/GLCommon.hpp"

#include <fstream>
#include <sstream>

#include "Rendering/AxisRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Object.hpp"
#include "GlobalSceneManager.hpp"

namespace Engine::Rendering
{
	AxisRenderer::AxisRenderer()
	{
		std::ifstream vert("data/shaders/axisrenderer.vert");
		std::ifstream frag("data/shaders/axisrenderer.frag");
		std::ostringstream vertsstr;
		std::ostringstream fragsstr;
		vertsstr << vert.rdbuf();
		fragsstr << frag.rdbuf();

		this->program = std::make_unique<Shader>(vertsstr.str().c_str(), fragsstr.str().c_str());
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


		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)this->_screenx / this->_screeny, 0.1f, 100.0f);
		glm::mat4 View = global_scene_manager->GetCameraViewMatrix();
		glm::mat4 Model = glm::mat4(1.0f);

		this->matMVP = Projection * View * Model;

		glNamedBufferData(this->vbo, sizeof(data_vert) + sizeof(data_axis), NULL, GL_STATIC_DRAW);
		glNamedBufferSubData(this->vbo, 0, sizeof(data_vert), (void*)&data_vert[0]);
		glNamedBufferSubData(this->vbo, sizeof(data_vert), sizeof(data_axis), (void*)&data_axis[0]);
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

		glUseProgram(shader);

		glBindVertexArray(this->vao);

		// Vertex and texture coord data
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);
		glVertexAttribIPointer(1, 1, GL_INT, 0, (void*)(18 * sizeof(GLfloat)));

		GLuint mvp_uniform_id = glGetUniformLocation(shader, "matMVP");
		glUniformMatrix4fv(mvp_uniform_id, 1, GL_FALSE, &this->matMVP[0][0]);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glLineWidth(5.0);
		glDrawArrays(GL_LINES, 0, 18);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
}