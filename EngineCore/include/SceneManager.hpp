#pragma once

#include "InternalEngineObject.hpp"
#include "Scene.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	class SceneLoader;

	class SceneManager final : public InternalEngineObject
	{
	public:
		float field_of_vision = initial_fov;

		SceneManager(Engine* with_engine);
		~SceneManager();

		void				   setSceneLoadPrefix(const std::string& scene_prefix);
		void				   loadScene(const std::string& scene_name);
		void				   setScene(const std::string& scene_name);
		std::shared_ptr<Scene> getSceneCurrent();

		/**
		 * @todo Remove?
		 */
		glm::vec3 getLightTransform();
		void	  setLightTransform(glm::vec3 newpos);

		glm::vec3 getCameraTransform();
		glm::vec2 getCameraRotation();
		glm::mat4 getCameraViewMatrix();

		[[nodiscard]] glm::mat4		   getProjectionMatrix() const;
		[[nodiscard]] static glm::mat4 getProjectionMatrixOrtho();

		void setCameraTransform(glm::vec3 transform);
		void setCameraRotation(glm::vec2 hvrotation);

	private:
		using scene_t = std::shared_ptr<Scene>;

		std::unordered_map<std::string, scene_t> scenes;
		scene_t									 current_scene;

		glm::vec3 camera_transform	 = {}; // XYZ position
		glm::vec2 camera_hv_rotation = {}; // Horizontal and Vertical rotation

		static constexpr float initial_fov = 45.0f;

		/**
		 * @todo Remove?
		 */
		glm::vec3 light_position{};

		std::unique_ptr<SceneLoader> scene_loader;

		void onCameraUpdated();
	};
} // namespace Engine
