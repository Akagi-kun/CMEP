#include "SceneLoader.hpp"

namespace Engine
{
    SceneLoader::SceneLoader(std::shared_ptr<Logging::Logger> logger)
    {
        this->logger = logger;
    }

    SceneLoader::~SceneLoader()
    {

    }

    static void LoadSceneInternal(std::shared_ptr<Scene>& scene)
    {

    }

    std::shared_ptr<Scene> SceneLoader::LoadScene(std::string scene_name)
    {
        std::shared_ptr<Scene> new_scene = std::make_shared<Scene>();
        new_scene->logger = this->logger;

        LoadSceneInternal(new_scene);

        return new_scene;
    }
}