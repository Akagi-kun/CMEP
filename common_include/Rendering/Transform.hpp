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

	template <typename T> struct Vector2
	{
		using self_t  = Vector2<T>;
		using value_t = T;

		T x;
		T y;
	};

	using ScreenSize  = Vector2<uint_fast16_t>;
	using TextureSize = Vector2<uint_fast32_t>;

} // namespace Engine::Rendering
