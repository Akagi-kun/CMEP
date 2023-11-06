#include "GlobalSceneManager.hpp"
#include "Logging/Logging.hpp"
#include "glm/common.hpp"
#include "Engine.hpp"

namespace Engine
{
	GlobalSceneManager::GlobalSceneManager()
	{
		// Reset transform and rotation
		this->cameraTransform = glm::vec3(2.0, 0, 1.0);
		this->cameraHVRotation = glm::vec2(0.0, 0.0);
	}

	GlobalSceneManager::~GlobalSceneManager()
	{
		for (auto& [name, ptr] : this->objects)
		{
			delete ptr;
		}
		this->objects.clear();
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
		return &(this->objects);
	}
	
	void GlobalSceneManager::AddObject(std::string name, Object* ptr)
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Adding object \"%s\" to globally managed scene", name.c_str());
		if (ptr != nullptr)
		{
			Rendering::GLFWwindowData data = global_engine->GetRenderingEngine()->GetWindow();
			ptr->ScreenSizeInform(data.windowX, data.windowY);
			this->objects.emplace(name, ptr);
		}
	}

	Object* GlobalSceneManager::FindObject(std::string name)
	{
		auto find_ret = this->objects.find(name);
		if (find_ret != this->objects.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	size_t GlobalSceneManager::RemoveObject(std::string name) noexcept
	{
		Object* object = this->FindObject(name);
		
		if(object)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Removing object \"%s\" from globally managed scene, deleting object", name.c_str());
			delete object;
		}

		return this->objects.erase(name);
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

	CMEP_EXPORT GlobalSceneManager* global_scene_manager = nullptr;
}