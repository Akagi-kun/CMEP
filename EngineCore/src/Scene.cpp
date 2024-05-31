#include "Scene.hpp"
#include "Engine.hpp"
#include "Rendering/SpriteRenderer.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
    Scene::Scene() {}
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
		//Scene::InternalSort(this->objects);

		return &(this->objects);
	}
	
	static bool InternalSort_CmpFunction(std::pair<std::string, Object*>& a, std::pair<std::string, Object*>& b)
	{
		const bool isA_UI = a.second->renderer->GetIsUI();
		const bool isB_UI = b.second->renderer->GetIsUI();

		if(isA_UI && isB_UI)
		{
			glm::vec3 a_pos = a.second->position();
			glm::vec3 b_pos = b.second->position();

			if(a_pos.z < b_pos.z)
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

	void Scene::InternalSort(std::unordered_map<std::string, Object*>& map)
	{
		std::vector<std::pair<std::string, Object*>> a;

		for (auto& it : map)
		{
			a.push_back(it);
		}

		std::sort(a.begin(), a.end(), InternalSort_CmpFunction);

		map.clear();

		for(auto& it : a)
		{
			map.insert(it);
		}
	}

	Object* Scene::AddTemplatedObject(std::string name, std::string template_name)
	{
		auto templated_object = this->templates.find(template_name);

		if(templated_object != this->templates.end())
		{
			Object* object = new Object();

			if(templated_object->second->renderer_type == "sprite")
			{
				object->renderer = new Rendering::SpriteRenderer(this->owner_engine);
				//*(Rendering::SpriteRenderer*)object->renderer = *(Rendering::SpriteRenderer*)templated_object->second->renderer;
				((Rendering::SpriteRenderer*)object->renderer)->texture = ((Rendering::SpriteRenderer*)templated_object->second->renderer)->texture;
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
		Scene::InternalSort(this->objects);
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
		
		if(object)
		{
			this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Removing object \"%s\" from scene, deleting object", name.c_str());
			delete object;
		}

		return this->objects.erase(name);
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
}