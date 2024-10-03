#pragma once

#include "Assets/AssetManager.hpp"

#include "InternalEngineObject.hpp"
#include "Scene.hpp"

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

		std::shared_ptr<Scene> loadScene(std::string name);

	protected:
		void loadSceneAsset(
			std::shared_ptr<AssetManager>& asset_manager,
			nlohmann::json&				   asset_entry,
			const std::filesystem::path&   scene_path
		);
		void loadSceneAssets(
			std::shared_ptr<AssetManager>& asset_manager,
			nlohmann::json&				   data,
			const std::filesystem::path&   scene_path
		);

		void loadSceneTemplates(
			std::shared_ptr<AssetManager>& asset_manager,
			nlohmann::json&				   data,
			std::shared_ptr<Scene>&		   scene
		);
		void loadSceneEventHandlers(
			std::shared_ptr<AssetManager>& asset_manager,
			nlohmann::json&				   data,
			std::shared_ptr<Scene>&		   scene
		);

		void loadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name);
	};
} // namespace Engine
