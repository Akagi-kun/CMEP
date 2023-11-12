#include "GlobalSceneManager.hpp"
#include "Logging/Logging.hpp"
#include "glm/common.hpp"
#include "Engine.hpp"

namespace Engine
{
	GlobalSceneManager::GlobalSceneManager(std::shared_ptr<Logging::Logger> logger)
	{
		// Reset transform and rotation
		this->cameraTransform = glm::vec3(2.0, 0, 1.0);
		this->cameraHVRotation = glm::vec2(0.0, 0.0);

		this->logger = logger;

		this->current_scene = std::make_unique<Scene>();
		this->current_scene->logger = this->logger;
	}

	GlobalSceneManager::~GlobalSceneManager()
	{
	}

	void GlobalSceneManager::CameraUpdated()
	{
		for (auto& [name, ptr] : this->objects)
		{
			ptr->renderer->UpdateMesh();
		}
	}

	const std::unordered_map<std::string, Object*>* const GlobalSceneManager::GetAllObjects() noexcept
	{
		return this->current_scene->GetAllObjects();
	}
	
	void GlobalSceneManager::AddObject(std::string name, Object* ptr)
	{
		this->current_scene->owner_engine = this->owner_engine;
		this->current_scene->AddObject(name, ptr);
	}

	Object* GlobalSceneManager::FindObject(std::string name)
	{
		return this->current_scene->FindObject(name);
	}

	size_t GlobalSceneManager::RemoveObject(std::string name) noexcept
	{
		return this->current_scene->RemoveObject(name);
	}

	glm::vec3 GlobalSceneManager::GetLightTransform()
	{
		return this->lightPosition;
	}

	void GlobalSceneManager::SetLightTransform(glm::vec3 newpos)
	{
		this->lightPosition = newpos;
	}

	glm::vec3 GlobalSceneManager::GetCameraTransform()
	{
		return this->cameraTransform;
	}

	glm::vec2 GlobalSceneManager::GetCameraHVRotation()
	{
		return this->cameraHVRotation;
	}

	glm::mat4 GlobalSceneManager::GetCameraViewMatrix()
	{
		glm::vec3 direction = glm::vec3(
			cos(this->cameraHVRotation.x) * cos(this->cameraHVRotation.y),
			sin(this->cameraHVRotation.y),
			sin(this->cameraHVRotation.x) * cos(this->cameraHVRotation.y)
		);

		glm::vec3 front = glm::normalize(direction);
		glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
		glm::vec3 up = glm::normalize(glm::cross(right, front));

		glm::mat4 ViewMatrix = glm::lookAt(this->cameraTransform, this->cameraTransform + front, up);
		
		return ViewMatrix;
	}

	void GlobalSceneManager::SetCameraTransform(glm::vec3 transform)
	{
		this->cameraTransform = transform;
		this->CameraUpdated();
	}

	void GlobalSceneManager::SetCameraHVRotation(glm::vec2 hvrotation)
	{
		if (hvrotation.y < 1.7f)
		{
			hvrotation.y = 1.7f;
		}
		else if (hvrotation.y > 4.5f)
		{
			hvrotation.y = 4.5f;
		}

		this->cameraHVRotation = hvrotation;
		this->CameraUpdated();
	}

	void GlobalSceneManager::UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
	{
		this->logger = new_logger;
	}
}