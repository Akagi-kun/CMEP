#pragma once

#include "Logging/Logging.hpp"

#include "InternalEngineObject.hpp"
#include "Scene.hpp"

#include <memory>

#ifndef JSON_USE_IMPLICIT_CONVERSIONS
#	define JSON_USE_IMPLICIT_CONVERSIONS 0
#endif
#include "nlohmann/json.hpp"

namespace Engine
{
	class SceneLoader : public InternalEngineObject
	{
	protected:
		void LoadSceneAssets(nlohmann::json& data, std::string& scene_path);
		void LoadSceneTemplates(nlohmann::json& data, std::shared_ptr<Scene>& scene);
		void LoadSceneTree(nlohmann::json& data, std::shared_ptr<Scene>& scene);

		void LoadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name);

		RendererType InterpretRendererType(nlohmann::json& from);

	public:
		std::string scene_prefix;

		SceneLoader(std::shared_ptr<Logging::Logger> with_logger);
		~SceneLoader();

		std::shared_ptr<Scene> LoadScene(std::string name);
	};
} // namespace Engine
