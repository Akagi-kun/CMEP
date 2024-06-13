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

	typedef glm::vec<2, uint16_t> ScreenSize;

} // namespace Engine::Rendering
