#pragma once

#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class CMEP_EXPORT Shader final
	{
	private:
		int program = 0;

		static unsigned int SetupShader(const char* vert, const char* frag) noexcept;

	public:
		Shader(const char* vert, const char* frag) noexcept;

		unsigned int GetProgram() const noexcept;

		bool IsValid() const noexcept;
	};
}