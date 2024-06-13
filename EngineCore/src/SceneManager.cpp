#include "SceneManager.hpp"

#include "Rendering/IRenderer.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace Engine
{
	SceneManager::SceneManager(std::shared_ptr<Logging::Logger> with_logger)
	{
		// Reset transform and rotation
		this->camera_transform = glm::vec3(0.0, 0.0, 0.0);
		this->camera_hv_rotation = glm::vec2(0.0, 0.0);

		this->logger = with_logger;

		this->scenes.emplace("_default", std::make_shared<Scene>());
		this->scenes.at(this->current_scene)->UpdateHeldLogger(this->logger);

		this->scene_loader = std::make_shared<SceneLoader>(with_logger);
	}

	SceneManager::~SceneManager()
	{
	}

	void SceneManager::CameraUpdated()
	{
		for (auto& [name, ptr] : *(this->scenes.at(this->current_scene)->GetAllObjects()))
		{
			Rendering::IRenderer* object_renderer = ptr->GetRenderer();
			assert(object_renderer != nullptr);

			object_renderer->UpdateMesh();
		}
	}

	void SceneManager::SetSceneLoadPrefix(std::string scene_prefix)
	{
		this->scene_loader->scene_prefix = scene_prefix;
	}

	void SceneManager::LoadScene(std::string scene_name)
	{
		this->scene_loader->UpdateOwnerEngine(this->owner_engine);
		this->scenes.emplace(scene_name, this->scene_loader->LoadScene(scene_name));
	}

	void SceneManager::SetScene(std::string scene_name)
	{
		this->current_scene = scene_name;
	}

	std::shared_ptr<Scene> SceneManager::GetSceneCurrent()
	{
		return this->scenes.at(this->current_scene);
	}

	const std::unordered_map<std::string, Object*>* SceneManager::GetAllObjects() noexcept
	{
		return this->scenes.at(this->current_scene)->GetAllObjects();
	}

	void SceneManager::AddObject(std::string name, Object* ptr)
	{
		this->scenes.at(this->current_scene)->UpdateOwnerEngine(this->owner_engine);
		this->scenes.at(this->current_scene)->AddObject(name, ptr);
	}

	Object* SceneManager::FindObject(std::string name)
	{
		return this->scenes.at(this->current_scene)->FindObject(name);
	}

	size_t SceneManager::RemoveObject(std::string name) noexcept
	{
		return this->scenes.at(this->current_scene)->RemoveObject(name);
	}

	Object* SceneManager::AddTemplatedObject(std::string name, std::string template_name)
	{
		return this->scenes.at(this->current_scene)->AddTemplatedObject(name, template_name);
	}

	glm::vec3 SceneManager::GetLightTransform()
	{
		return this->light_position;
	}

	void SceneManager::SetLightTransform(glm::vec3 newpos)
	{
		this->light_position = newpos;
	}

	glm::vec3 SceneManager::GetCameraTransform()
	{
		return this->camera_transform;
	}

	glm::vec2 SceneManager::GetCameraHVRotation()
	{
		return this->camera_hv_rotation;
	}

	glm::mat4 SceneManager::GetCameraViewMatrix()
	{
		glm::vec3 direction = glm::vec3(
			cos(this->camera_hv_rotation.x) * cos(this->camera_hv_rotation.y),
			sin(this->camera_hv_rotation.y),
			sin(this->camera_hv_rotation.x) * cos(this->camera_hv_rotation.y)
		);

		glm::vec3 front = glm::normalize(direction);
		glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
		glm::vec3 up = glm::normalize(glm::cross(right, front));

		glm::mat4 view_matrix = glm::lookAt(this->camera_transform, this->camera_transform + front, up);

		return view_matrix;
	}

	void SceneManager::SetCameraTransform(glm::vec3 transform)
	{
		this->camera_transform = transform;
		this->CameraUpdated();
	}

	void SceneManager::SetCameraHVRotation(glm::vec2 hvrotation)
	{
		static const float rotation_y_limit = 1.7f;
		static const float rotation_x_limit = 4.5f;

		if (hvrotation.y < rotation_y_limit)
		{
			hvrotation.y = rotation_y_limit;
		}
		else if (hvrotation.y > rotation_x_limit)
		{
			hvrotation.y = rotation_x_limit;
		}

		this->camera_hv_rotation = hvrotation;
		this->CameraUpdated();
	}
} // namespace Engine
