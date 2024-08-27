#pragma once

#include "glm/glm.hpp"

#include <cstdint>

namespace Engine::Rendering
{
	struct Transform
	{
		glm::vec3 pos	   = glm::vec3();
		glm::vec3 size	   = glm::vec3();
		glm::vec3 rotation = glm::vec3();
	};

	// Screen size, use for windows etc
	using ScreenSize = glm::vec<2, uint_fast16_t>;

	// Texture size
	using ImageSize = glm::vec<2, uint_fast32_t>;

} // namespace Engine::Rendering
