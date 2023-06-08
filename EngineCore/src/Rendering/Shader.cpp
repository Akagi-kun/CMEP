#include <assert.h>

#include "Rendering/Shader.hpp"

namespace Engine::Rendering
{
	GLuint Shader::SetupShader(const char* vert_src, const char* frag_src) noexcept
	{
		assert(vert_src != nullptr && frag_src != nullptr);
		GLuint program = 0;

		unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		assert(vertex_shader != 0);
		glShaderSource(vertex_shader, 1, &(vert_src), NULL);
		glCompileShader(vertex_shader);

		unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		assert(fragment_shader != 0);
		glShaderSource(fragment_shader, 1, &(frag_src), NULL);
		glCompileShader(fragment_shader);
		program = glCreateProgram();
		assert(program != 0);

		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);

		glLinkProgram(program);

		glDetachShader(program, vertex_shader);
		glDetachShader(program, fragment_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	
		return program;
	}

	Shader::Shader(const char* vert_src, const char* frag_src) noexcept
	{
		this->program = Shader::SetupShader(vert_src, frag_src);
	}

	GLuint Shader::GetProgram() const noexcept
	{
		return this->program;
	}

	bool Shader::IsValid() const noexcept
	{
		return this->program != 0;
	}
}