#pragma once

#include "Rendering/Transform.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Engine::Rendering
{
	[[nodiscard]] inline glm::mat4 calculateModelMatrix(
		const Transform& local_transform,
		const Transform& parent_transform
	)
	{
		glm::quat model_rotation  = glm::quat(glm::radians(local_transform.rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(parent_transform.rotation));

		glm::mat4 model = glm::scale(
			glm::translate(
				glm::scale(
					glm::translate(glm::identity<glm::mat4>(), parent_transform.pos) *
						glm::mat4_cast(parent_rotation),
					parent_transform.size
				),
				local_transform.pos
			) * glm::mat4_cast(model_rotation),
			local_transform.size
		);

		return model;
	}
} // namespace Engine::Rendering
