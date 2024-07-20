#pragma once

#include "InternalEngineObject.hpp"
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
		std::string current_scene_name = "_default";

		glm::vec3 camera_transform{};	// XYZ position
		glm::vec2 camera_hv_rotation{}; // Horizontal and Vertical rotation

		glm::vec3 light_position{};

		static constexpr float initial_fov = 45.0f;
		float field_of_vision			   = initial_fov;

		void OnCameraUpdated();

		std::unique_ptr<SceneLoader> scene_loader;

	public:
		SceneManager(Engine* with_engine);
		~SceneManager();

		void SetSceneLoadPrefix(const std::string& scene_prefix);
		void LoadScene(std::string scene_name);
		void SetScene(const std::string& scene_name);
		std::shared_ptr<Scene>& GetSceneCurrent();

		glm::vec3 GetLightTransform();
		void SetLightTransform(glm::vec3 newpos);

		glm::vec3 GetCameraTransform();
		glm::vec2 GetCameraHVRotation();
		glm::mat4 GetCameraViewMatrix();

		[[nodiscard]] glm::mat4 GetProjectionMatrix(Rendering::ScreenSize screen) const;
		[[nodiscard]] static glm::mat4 GetProjectionMatrixOrtho();

		void SetCameraTransform(glm::vec3 transform);
		void SetCameraHVRotation(glm::vec2 hvrotation);
	};
} // namespace Engine
