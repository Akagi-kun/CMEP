#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Object.hpp"
#include "PlatformSemantics.hpp"

#include "Logging/Logging.hpp"

#include <unordered_map>
#include <memory>

namespace Engine
{
	class CMEP_EXPORT GlobalSceneManager final
	{
	private:
		std::unordered_map<std::string, Object*> objects{};

		glm::vec3 cameraTransform{}; // XYZ position
		glm::vec2 cameraHVRotation{}; // Horizontal and Vertical rotation

		glm::vec3 lightPosition{};

		void CameraUpdated();
	public:
		std::shared_ptr<Logging::Logger> logger;

		GlobalSceneManager();
		~GlobalSceneManager();

		void UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger);

		const std::unordered_map<std::string, Object*>* const GetAllObjects() noexcept;

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;

		glm::vec3 GetLightTransform();
		void SetLightTransform(glm::vec3 newpos);

		glm::vec3 GetCameraTransform();
		glm::vec2 GetCameraHVRotation();
		glm::mat4 GetCameraViewMatrix();

		void SetCameraTransform(glm::vec3 transform);
		void SetCameraHVRotation(glm::vec2 hvrotation);
	};
}
