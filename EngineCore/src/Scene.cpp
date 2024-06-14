#include "Scene.hpp"

// #include "Assets/AssetManager.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"

#include "Engine.hpp"

#include <exception>

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
		this->templates.clear();

		for (auto& [name, ptr] : this->objects)
		{
			delete ptr;
		}
		this->objects.clear();
		this->objects_sorted.clear();
	}

	const std::unordered_map<std::string, Object*>* Scene::GetAllObjects() noexcept
	{
		return &(this->objects);
	}

	const std::vector<std::pair<std::string, Object*>>* Scene::GetAllObjectsSorted() noexcept
	{
		return &(this->objects_sorted);
	}

	static bool InternalSortCmpFunction(std::pair<std::string, Object*>& a, std::pair<std::string, Object*>& b)
	{
		if (a.second == nullptr || b.second == nullptr)
		{
			throw std::exception("Could not sort scene, object is nullptr!");
		}
		assert(a.second != nullptr);
		assert(b.second != nullptr);

		Rendering::IRenderer* a_renderer = a.second->GetRenderer();
		Rendering::IRenderer* b_renderer = b.second->GetRenderer();

		assert(a_renderer != nullptr);
		assert(b_renderer != nullptr);

		const bool is_a_ui = a_renderer->GetIsUI();
		const bool is_b_ui = b_renderer->GetIsUI();

		if (is_a_ui && is_b_ui)
		{
			// Introduce positive offset (TODO: is this necessary?)
			static const float positive_offset = 10.f;
			float a_z = a.second->Position().z + positive_offset;
			float b_z = b.second->Position().z + positive_offset;

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
		std::unordered_map<std::string, Object*>& from_map,
		std::vector<std::pair<std::string, Object*>>& objects
	)
	{
		std::vector<std::pair<std::string, Object*>> a;

		for (auto& it : from_map)
		{
			assert(it.second != nullptr);
			a.emplace_back(it);
		}

		std::sort(a.begin(), a.end(), InternalSortCmpFunction);

		objects.clear();

		for (auto& it : a)
		{
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
			// Object* object = nullptr;
			Rendering::IRenderer* with_renderer = nullptr;

			ObjectTemplate object_template = templated_object->second;

			switch (object_template.with_renderer)
			{
				case RendererType::SPRITE:
				{
					// object = new Object();
					with_renderer = new Rendering::SpriteRenderer(this->owner_engine);
					// object->UpdateHeldLogger(this->logger);
					with_renderer->UpdateHeldLogger(this->logger);

					break;
				}
				default:
				{
					// Unknown renderer type, cannot add object
					return nullptr;
				}
			}

			// Allocate Object since we already know
			// that the renderer is valid
			auto* object = new Object();

			for (auto& supply : object_template.supply_list)
			{
				with_renderer->SupplyData(supply);
			}

			auto* old_renderer = object->AssignRenderer(with_renderer);
			assert(old_renderer == nullptr);

			object->UpdateHeldLogger(this->logger);
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
			ptr->ScreenSizeInform(data.window_x, data.window_y);
			ptr->UpdateHeldLogger(this->logger);
			this->objects.emplace(name, ptr);
		}

		try
		{
			Scene::InternalSort(this->objects, this->objects_sorted);
		}
		catch (std::exception& e)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Info, LOGPFX_CURRENT "Exception sorting objects! e.what(): %s", e.what()
			);
			throw;
		}
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

	size_t Scene::RemoveObject(std::string name)
	{
		Object* object = this->FindObject(name);

		if (object)
		{
			this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Removing object '%s'", name.c_str());
			delete object;
		}

		size_t return_val = this->objects.erase(name);

		// TODO: Bad performance? a resort should be done after all removals
		try
		{
			Scene::InternalSort(this->objects, this->objects_sorted);
		}
		catch (std::exception& e)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Info, LOGPFX_CURRENT "Exception sorting objects! e.what(): %s", e.what()
			);
			throw;
		}

		return return_val;
	}

	void Scene::LoadTemplatedObject(std::string name, ObjectTemplate object)
	{
		this->templates.emplace(name, object);
	}
} // namespace Engine
