#include "Scene.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/Vulkan/backend.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "Exception.hpp"
#include "InternalEngineObject.hpp"
#include "SceneObject.hpp"

#include <format>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	Scene::Scene(Engine* with_engine)
		: InternalEngineObject(with_engine), asset_repository(new AssetRepository())
	{}

	Scene::~Scene()
	{
		this->logger->logSingle<decltype(this)>(Logging::LogLevel::VerboseDebug, "Destructor called");

		templates.clear();

		for (auto& [name, ptr] : objects)
		{
			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Deleting object '{}'",
				name
			);
			delete ptr;
		}

		objects.clear();
	}

	const std::unordered_map<std::string, SceneObject*>& Scene::getAllObjects() noexcept
	{
		return objects;
	}

	void Scene::addTemplatedObject(const std::string& name, const std::string& template_name)
	{
		auto templated_object = templates.find(template_name);

		if (templated_object != templates.end())
		{
			SceneObject* obj = Factories::ObjectFactory::instantiateObjectTemplate(
				getOwnerEngine(),
				templated_object->second
			);

			addObject(name, obj);
		}
		else
		{
			throw ENGINE_EXCEPTION(
				std::format("Template with name '{}' could not be found!", template_name)
			);
		}
	}

	void Scene::addObject(const std::string& name, SceneObject* ptr)
	{
		if (ptr == nullptr)
		{
			throw ENGINE_EXCEPTION("Called AddObject with nullptr!");
		}

		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Debug, "Adding object '{}'", name);

			objects.emplace(name, ptr);
	}

	SceneObject* Scene::findObject(const std::string& name)
	{
		auto find_ret = objects.find(name);
		if (find_ret != objects.end()) { return find_ret->second; }
		return nullptr;
	}

	void Scene::removeObject(const std::string& name)
	{
		SceneObject* object = findObject(name);

		if (object != nullptr)
		{
			objects.erase(name);
			delete object;
		}
		else
		{
			throw ENGINE_EXCEPTION(std::format("Could not remove non-existent object '{}'!", name));
		}
	}

	void Scene::loadTemplatedObject(
		const std::string&								name,
		const Factories::ObjectFactory::ObjectTemplate& object
	)
	{
		templates.emplace(name, object);
	}
} // namespace Engine
