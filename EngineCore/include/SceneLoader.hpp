#pragma once

#include "Scene.hpp"
#include "Logging/Logging.hpp"

#include <memory>

namespace Engine
{
    class SceneLoader
    {
    protected:
    public:
		std::shared_ptr<Logging::Logger> logger;
		Engine* owner_engine;

        std::string scene_prefix;

        SceneLoader(std::shared_ptr<Logging::Logger> logger);
        ~SceneLoader();

        std::shared_ptr<Scene> LoadScene(std::string name);
    };
}