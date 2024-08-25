#include "SceneManager.hpp"

#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "Exception.hpp"
#include "InternalEngineObject.hpp"
#include "SceneLoader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE_MANAGER
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine
{
	SceneManager::SceneManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
		// Reset transform and rotation
		this->logger = owner_engine->GetLogger();

		std::shared_ptr<Scene> default_scene = std::make_shared<Scene>(with_engine);

		scenes.emplace("_default", default_scene);

		scene_loader = std::make_unique<SceneLoader>(with_engine);

		// Reset transform and rotation
		SetCameraHVRotation({0.0, 0.0});
		SetCameraTransform({0.0, 0.0, 0.0});
	}

	SceneManager::~SceneManager()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		scenes.clear();

		scene_loader.reset();
	}

	void SceneManager::OnCameraUpdated()
	{
		auto& current_scene = scenes.at(current_scene_name);

		// Explicitly update matrices of all objects
		// since otherwise they'd update them only on transform updates
		for (const auto& [name, ptr] : current_scene->GetAllObjects())
		{
			auto* object_renderer = static_cast<Rendering::IRenderer*>(ptr->GetRenderer());
			assert(object_renderer != nullptr);

			object_renderer->UpdateMatrices();
		}
	}

	void SceneManager::SetSceneLoadPrefix(const std::string& scene_prefix)
	{
		scene_loader->scene_prefix = scene_prefix;
	}

	void SceneManager::LoadScene(std::string scene_name)
	{
		assert(owner_engine != nullptr);

		try
		{
			scenes.emplace(scene_name, scene_loader->LoadScene(scene_name));
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Could not load scene"));
		}
	}

	void SceneManager::SetScene(const std::string& scene_name)
	{
		current_scene_name = scene_name;
	}

	std::shared_ptr<Scene>& SceneManager::GetSceneCurrent()
	{
		return scenes.at(current_scene_name);
	}

	glm::vec3 SceneManager::GetLightTransform()
	{
		return light_position;
	}

	void SceneManager::SetLightTransform(glm::vec3 newpos)
	{
		light_position = newpos;
	}

	glm::vec3 SceneManager::GetCameraTransform()
	{
		return camera_transform;
	}

	glm::vec2 SceneManager::GetCameraHVRotation()
	{
		return camera_hv_rotation;
	}

	glm::mat4 SceneManager::GetProjectionMatrix(Rendering::ScreenSize screen) const
	{
		static constexpr float nearplane = 0.1f;
		static constexpr float farplane	 = 1000.0f;

		return glm::perspective<float>(
			glm::radians(field_of_vision),
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
		auto pitch = glm::radians(camera_hv_rotation.y);
		auto yaw   = glm::radians(camera_hv_rotation.x);

		// Points forward
		glm::vec3 direction = {
			cos(yaw) * cos(pitch),
			sin(pitch),
			sin(yaw) * cos(pitch),
		};
		glm::vec3 forward = glm::normalize(direction);

		// Points to the right
		glm::vec3 right	   = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
		// Points up
		glm::vec3 up_local = glm::normalize(glm::cross(right, forward));

		glm::mat4 view_matrix = glm::lookAt(camera_transform, camera_transform + forward, up_local);

		return view_matrix;
	}

	void SceneManager::SetCameraTransform(glm::vec3 transform)
	{
		camera_transform = transform;
		OnCameraUpdated();
	}

	void SceneManager::SetCameraHVRotation(glm::vec2 hvrotation)
	{
		static constexpr float y_center = 180.f;
		static constexpr float y_min	= y_center - 90.f;
		static constexpr float y_max	= y_center + 89.9f;

		// Account for rotation possibly being NaN
		if (std::isnan(hvrotation.y))
		{
			hvrotation.y = y_center;
		}
		if (std::isnan(hvrotation.x))
		{
			hvrotation.x = 0;
		}

		// Clamp Y so you cannot do a backflip
		hvrotation.y = std::clamp(hvrotation.y, y_min, y_max);

		static constexpr float x_min = 0.f;
		static constexpr float x_max = 360.f;

		// Wrap X around (so we don't get awkwardly high or low X values)
		if (hvrotation.x > x_max)
		{
			hvrotation.x = x_min;
		}
		else if (hvrotation.x < x_min)
		{
			hvrotation.x = x_max;
		}

		camera_hv_rotation = hvrotation;
		OnCameraUpdated();
	}
} // namespace Engine
