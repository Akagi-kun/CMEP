#include "Scene.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "Object.hpp"

#include <memory>
#include <stdexcept>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine
{
	Scene::~Scene()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Destructor called");

		this->templates.clear();

		for (auto& [name, ptr] : this->objects)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Deleting object '%s'", name.c_str());
			delete ptr;
		}

		this->objects.clear();
	}

	const std::unordered_map<std::string, Object*>& Scene::GetAllObjects() noexcept
	{
		return this->objects;
	}

	void Scene::AddTemplatedObject(const std::string& name, const std::string& template_name)
	{
		auto templated_object = this->templates.find(template_name);

		if (templated_object != this->templates.end())
		{
			Factories::ObjectFactory::ObjectTemplate object_template = templated_object->second;

			Object* obj = Factories::ObjectFactory::InstantiateObjectTemplate(this->GetOwnerEngine(), object_template);

			this->AddObject(name, obj);
		}
		else
		{
			throw std::invalid_argument("Template with name '" + template_name + "' could not be found!");
		}
	}

	void Scene::AddObject(const std::string& name, Object* ptr)
	{
		if (ptr != nullptr)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Adding object '%s'", name.c_str());

			Rendering::GLFWwindowData window_data = this->owner_engine->GetRenderingEngine()->GetWindow();
			ptr->ScreenSizeInform(window_data.size.x, window_data.size.y);

			this->objects.emplace(name, ptr);
		}
		else
		{
			throw std::invalid_argument("Called AddObject with nullptr!");
		}
	}

	Object* Scene::FindObject(const std::string& name)
	{
		auto find_ret = this->objects.find(name);
		if (find_ret != this->objects.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	void Scene::RemoveObject(const std::string& name)
	{
		Object* object = this->FindObject(name);

		if (object != nullptr)
		{
			this->objects.erase(name);
			delete object;
		}
		else
		{
			throw std::invalid_argument("Could not remove non-existent object '" + name + "'!");
		}
	}

	void Scene::LoadTemplatedObject(const std::string& name, Factories::ObjectFactory::ObjectTemplate object)
	{
		this->templates.emplace(name, object);
	}
} // namespace Engine
