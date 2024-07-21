#include "SceneManager.hpp"

#include "Rendering/IRenderer.hpp"
#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "InternalEngineObject.hpp"
#include "SceneLoader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	SceneManager::SceneManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
		// Reset transform and rotation
		this->camera_transform	 = glm::vec3(0.0, 0.0, 0.0);
		this->camera_hv_rotation = glm::vec2(0.0, 0.0);
		this->logger			 = this->owner_engine->GetLogger();

		std::shared_ptr<Scene> default_scene = std::make_shared<Scene>(with_engine);

		this->scenes.emplace("_default", default_scene);

		this->scene_loader = std::make_unique<SceneLoader>(with_engine);
	}

	SceneManager::~SceneManager()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");
		/*
		for (auto& scene : this->scenes)
		{
			scene.second.reset();
		} */
		this->scenes.clear();

		this->scene_loader.reset();
	}

	void SceneManager::OnCameraUpdated()
	{
		auto& current_scene = this->scenes.at(this->current_scene_name);

		for (const auto& [name, ptr] : current_scene->GetAllObjects())
		{
			auto* object_renderer = static_cast<Rendering::IRenderer*>(ptr->GetRenderer());
			assert(object_renderer != nullptr);

			object_renderer->UpdateMesh();
		}
	}

	void SceneManager::SetSceneLoadPrefix(const std::string& scene_prefix)
	{
		this->scene_loader->scene_prefix = scene_prefix;
	}

	void SceneManager::LoadScene(std::string scene_name)
	{
		assert(this->owner_engine != nullptr);

		this->scenes.emplace(scene_name, this->scene_loader->LoadScene(scene_name));
	}

	void SceneManager::SetScene(const std::string& scene_name)
	{
		this->current_scene_name = scene_name;
	}

	std::shared_ptr<Scene>& SceneManager::GetSceneCurrent()
	{
		return this->scenes.at(this->current_scene_name);
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

	glm::mat4 SceneManager::GetProjectionMatrix(Rendering::ScreenSize screen) const
	{
		static constexpr float nearplane = 0.1f;
		static constexpr float farplane	 = 100.0f;

		return glm::perspective<float>(
			glm::radians(this->field_of_vision),
			static_cast<float>(screen.x) / static_cast<float>(screen.y),
			nearplane,
			farplane
		);
	}

	glm::mat4 SceneManager::GetProjectionMatrixOrtho()
	{
		return glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	}

	glm::mat4 SceneManager::GetCameraViewMatrix()
	{
		glm::vec3 direction = glm::vec3(
			cos(this->camera_hv_rotation.x) * cos(this->camera_hv_rotation.y),
			sin(this->camera_hv_rotation.y),
			sin(this->camera_hv_rotation.x) * cos(this->camera_hv_rotation.y)
		);

		// Points forward
		glm::vec3 dir_forward = glm::normalize(direction);
		// Points to the right
		glm::vec3 dir_right	  = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
		// Points up
		glm::vec3 dir_up	  = glm::normalize(glm::cross(dir_right, dir_forward));

		glm::mat4 view_matrix = glm::lookAt(this->camera_transform, this->camera_transform + dir_forward, dir_up);

		return view_matrix;
	}

	void SceneManager::SetCameraTransform(glm::vec3 transform)
	{
		this->camera_transform = transform;
		this->OnCameraUpdated();
	}

	void SceneManager::SetCameraHVRotation(glm::vec2 hvrotation)
	{
		// TODO: check limit correctness
		static constexpr float rotation_y_limit = 1.7f;
		static constexpr float rotation_x_limit = 4.5f;

		// Limit rotation on Y
		if (hvrotation.y < rotation_y_limit)
		{
			hvrotation.y = rotation_y_limit;
		}
		// Limit rotation on X
		if (hvrotation.x > rotation_x_limit)
		{
			hvrotation.x = rotation_x_limit;
		}

		this->camera_hv_rotation = hvrotation;
		this->OnCameraUpdated();
	}
} // namespace Engine
