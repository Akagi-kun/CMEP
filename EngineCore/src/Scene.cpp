#include "Scene.hpp"
#include "Engine.hpp"
#include "Rendering/SpriteRenderer.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	Scene::Scene()
	{
	}
	Scene::~Scene()
	{
		for (auto& [name, ptr] : this->objects)
		{
			delete ptr;
		}
		this->objects.clear();

		for (auto& [name, ptr] : this->templates)
		{
			delete ptr;
		}
		this->templates.clear();
	}

	const std::unordered_map<std::string, Object*>* const Scene::GetAllObjects() noexcept
	{
		return &(this->objects);
	}

	const std::vector<std::pair<std::string, Object*>>* const Scene::GetAllObjectsSorted() noexcept
	{
		return &(this->objects_sorted);
	}

	static bool InternalSort_CmpFunction(std::pair<std::string, Object*>& a, std::pair<std::string, Object*>& b)
	{
		const bool isA_UI = a.second->renderer->GetIsUI();
		const bool isB_UI = b.second->renderer->GetIsUI();

		if (isA_UI && isB_UI)
		{
			float a_z = a.second->position().z + 10.0f;
			float b_z = b.second->position().z + 10.0f;
			/*
						if ((a.first.rfind("sprite_pipe", 0) == 0 || b.first.rfind("sprite_pipe", 0) == 0) &&
							(a.first.rfind("background", 0) || b.first.rfind("background", 0)))
						{
							printf("%s %lf %s %lf\n", a.first.c_str(), a_z, b.first.c_str(), b_z);
						} */

			if (a_z < b_z)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return std::less<std::pair<std::string, Object*>>{}(a, b);
		}
	}

	void Scene::InternalSort(
		std::unordered_map<std::string, Object*> from_map, std::vector<std::pair<std::string, Object*>>& objects
	)
	{
		std::vector<std::pair<std::string, Object*>> a;

		for (auto& it : from_map)
		{
			a.push_back(it);
		}

		// printf("\n");

		std::sort(a.begin(), a.end(), InternalSort_CmpFunction);

		// printf("sorted\n");

		objects.clear();

		for (auto& it : a)
		{
			// printf("%s\t\t%lf\n", it.first.c_str(), it.second->position().z);
			objects.push_back(it);
		}
	}

	void Scene::TriggerResort()
	{
		Scene::InternalSort(this->objects, this->objects_sorted);
	}

	Object* Scene::AddTemplatedObject(std::string name, std::string template_name)
	{
		auto templated_object = this->templates.find(template_name);

		if (templated_object != this->templates.end())
		{
			Object* object = new Object();

			if (templated_object->second->renderer_type == "sprite")
			{
				object->renderer = new Rendering::SpriteRenderer(this->owner_engine);
				//*(Rendering::SpriteRenderer*)object->renderer =
				//*(Rendering::SpriteRenderer*)templated_object->second->renderer;
				((Rendering::SpriteRenderer*)object->renderer)->texture =
					((Rendering::SpriteRenderer*)templated_object->second->renderer)->texture;
				object->UpdateHeldLogger(this->logger);
				object->renderer->UpdateHeldLogger(this->logger);
			}
			this->AddObject(name, object);

			return object;
		}
		return nullptr;
	}

	void Scene::AddObject(std::string name, Object* ptr)
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Adding object \"%s\"", name.c_str());
		if (ptr != nullptr)
		{
			Rendering::GLFWwindowData data = this->owner_engine->GetRenderingEngine()->GetWindow();
			ptr->ScreenSizeInform(data.windowX, data.windowY);
			ptr->UpdateHeldLogger(this->logger);
			ptr->renderer->UpdateHeldLogger(this->logger);
			this->objects.emplace(name, ptr);
		}
		Scene::InternalSort(this->objects, this->objects_sorted);
	}

	Object* Scene::FindObject(std::string name)
	{
		auto find_ret = this->objects.find(name);
		if (find_ret != this->objects.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	size_t Scene::RemoveObject(std::string name) noexcept
	{
		Object* object = this->FindObject(name);

		if (object)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Info,
				LOGPFX_CURRENT "Removing object \"%s\" from scene, deleting object",
				name.c_str()
			);
			delete object;
		}

		size_t returnVal = this->objects.erase(name);

		// TODO: Bad performance? a resort should be done after all removals
		Scene::InternalSort(this->objects, this->objects_sorted);

		return returnVal;
	}

	void Scene::LoadTemplatedObject(std::string name, Object* ptr)
	{
		if (ptr != nullptr)
		{
			Rendering::GLFWwindowData data = this->owner_engine->GetRenderingEngine()->GetWindow();
			ptr->ScreenSizeInform(data.windowX, data.windowY);
			ptr->UpdateHeldLogger(this->logger);
			ptr->renderer->UpdateHeldLogger(this->logger);
			this->templates.emplace(name, ptr);
		}
	}
} // namespace Engine