#pragma once

#include "glm/glm.hpp"

#include <cstdint>

namespace Engine::Rendering
{
	struct Transform
	{
		glm::vec3 pos = glm::vec3();
		glm::vec3 size = glm::vec3();
		glm::vec3 rotation = glm::vec3();
	};

	struct ScreenSize
	{
		uint_fast16_t x;
		uint_fast16_t y;
	};

	// typedef glm::vec<2, uint16_t> ScreenSize;

} // namespace Engine::Rendering
