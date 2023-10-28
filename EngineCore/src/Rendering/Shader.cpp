#include <vector>

#include "Logging/Logging.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/VulkanRenderingEngine.hpp"

namespace Engine::Rendering
{
	unsigned int Shader::SetupShader(const char* vert_src, const char* frag_src) noexcept
	{
		int program = 0;
		/*
		unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &(vert_src), NULL);
		glCompileShader(vertex_shader);

		GLint isCompiled = 0;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);

			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vertex shader compilation error:\n%s", &errorLog[0]);

			glDeleteShader(vertex_shader); // Don't leak the shader.
			return 0;
		}

		unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &(frag_src), NULL);
		glCompileShader(fragment_shader);

		isCompiled = 0;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);

			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Fragment shader compilation error:\n%s", &errorLog[0]);

			// Exit with failure.
			glDeleteShader(fragment_shader); // Don't leak the shader.
			return 0;
		}

		program = glCreateProgram();

		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);

		glLinkProgram(program);

		glDetachShader(program, vertex_shader);
		glDetachShader(program, fragment_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	*/
		return program;
	}

	Shader::Shader(const char* vert_src, const char* frag_src) noexcept
	{
		//this->program = Shader::SetupShader(vert_src, frag_src);
	}

	unsigned int Shader::GetProgram() const noexcept
	{
		return 0;
	}

	bool Shader::IsValid() const noexcept
	{
		return 0;
	}
}