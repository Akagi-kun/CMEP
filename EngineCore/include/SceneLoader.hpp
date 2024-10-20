#pragma once

#include "InternalEngineObject.hpp"
#include "Scene.hpp"

#include <filesystem>
#include <memory>
#include <string>

#ifndef JSON_USE_IMPLICIT_CONVERSIONS
#	define JSON_USE_IMPLICIT_CONVERSIONS 0
#endif
#include "nlohmann/json.hpp"

namespace Engine
{
	class SceneLoader : public InternalEngineObject
	{
	public:
		std::filesystem::path scene_prefix;

		using InternalEngineObject::InternalEngineObject;
		~SceneLoader();

		std::shared_ptr<Scene> loadScene(const std::string& name);

	protected:
		void loadSceneAssets(
			const nlohmann::json&		 data,
			const std::filesystem::path& scene_path,
			std::shared_ptr<Scene>&		 scene
		);

		void loadSceneTemplates(const nlohmann::json& data, std::shared_ptr<Scene>& scene);
		void loadSceneEventHandlers(const nlohmann::json& data, std::shared_ptr<Scene>& scene);

		void loadSceneInternal(std::shared_ptr<Scene>& scene, const std::string& scene_name);
	};
} // namespace Engine
