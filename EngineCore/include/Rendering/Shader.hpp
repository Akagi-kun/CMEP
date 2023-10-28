#pragma once

namespace Engine::Rendering
{
	class __declspec(dllexport) Shader final
	{
	private:
//		GLuint program = 0;

		static unsigned int SetupShader(const char* vert, const char* frag) noexcept;

	public:
		Shader(const char* vert, const char* frag) noexcept;

		unsigned int GetProgram() const noexcept;

		bool IsValid() const noexcept;
	};
}