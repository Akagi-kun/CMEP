#pragma once

#include "Rendering/Transform.hpp"

#include "glm/gtc/quaternion.hpp"

namespace Engine::Rendering
{
	[[nodiscard]] inline glm::mat4 CalculateModelMatrix(Transform local_transform, Transform parent_transform)
	{
		glm::quat model_rotation  = glm::quat(glm::radians(local_transform.rotation));
		glm::quat parent_rotation = glm::quat(glm::radians(parent_transform.rotation));
		glm::mat4 model			  = glm::scale(
			  glm::translate(
				  glm::scale(
					  glm::translate(glm::mat4(1.0f), parent_transform.pos) * glm::mat4_cast(parent_rotation),
					  parent_transform.size
				  ),
				  local_transform.pos
			  ) * glm::mat4_cast(model_rotation),
			  local_transform.size
		  );

		return model;
	}
} // namespace Engine::Rendering
