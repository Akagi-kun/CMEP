#pragma once

#include "Logging/Logging.hpp"

#include "InternalEngineObject.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "SceneLoader.hpp"

#include <memory>
#include <unordered_map>

namespace Engine
{
	class SceneManager final : public InternalEngineObject
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;
		std::string current_scene = "_default";

		glm::vec3 camera_transform{};	// XYZ position
		glm::vec2 camera_hv_rotation{}; // Horizontal and Vertical rotation

		glm::vec3 light_position{};

		void OnCameraUpdated();

		std::unique_ptr<SceneLoader> scene_loader;

	public:
		SceneManager(std::shared_ptr<Logging::Logger> logger);
		~SceneManager();

		void SetSceneLoadPrefix(std::string scene_prefix);
		void LoadScene(std::string scene_name);
		void SetScene(std::string scene_name);
		std::shared_ptr<Scene> GetSceneCurrent();

		// TODO: Remove Scene functions from here
		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		void RemoveObject(std::string name) noexcept;
		void AddTemplatedObject(std::string name, std::string template_name);

		glm::vec3 GetLightTransform();
		void SetLightTransform(glm::vec3 newpos);

		glm::vec3 GetCameraTransform();
		glm::vec2 GetCameraHVRotation();
		glm::mat4 GetCameraViewMatrix();

		void SetCameraTransform(glm::vec3 transform);
		void SetCameraHVRotation(glm::vec2 hvrotation);
	};
} // namespace Engine
