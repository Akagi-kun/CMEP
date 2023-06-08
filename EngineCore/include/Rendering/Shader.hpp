#pragma once

#include "Rendering/GLCommon.hpp"

namespace Engine::Rendering
{
	class __declspec(dllexport) Shader final
	{
	private:
		GLuint program = 0;

		static GLuint SetupShader(const char* vert, const char* frag) noexcept;

	public:
		Shader(const char* vert, const char* frag) noexcept;

		GLuint GetProgram() const noexcept;

		bool IsValid() const noexcept;
	};
}