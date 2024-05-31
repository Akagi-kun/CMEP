#pragma once

#include "Scene.hpp"
#include "Logging/Logging.hpp"
#include "InternalEngineObject.hpp"

#include <memory>

#ifndef JSON_USE_IMPLICIT_CONVERSIONS
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#endif
#include "nlohmann-json/single_include/nlohmann/json.hpp"

namespace Engine
{
    class SceneLoader : public InternalEngineObject
    {
    protected:
        void LoadSceneAssets(nlohmann::json& data, std::string& scene_path);

        void LoadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name);

    public:
        std::string scene_prefix;

        SceneLoader(std::shared_ptr<Logging::Logger> logger);
        ~SceneLoader();

        std::shared_ptr<Scene> LoadScene(std::string name);
    };
}