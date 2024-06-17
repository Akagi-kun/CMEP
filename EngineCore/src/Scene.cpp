#include "Scene.hpp"

// #include "Assets/AssetManager.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"

#include "Engine.hpp"
#include "IModule.hpp"

#include <exception>
#include <memory>
#include <stdexcept>

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
		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Destructor called");

		this->templates.clear();
		this->objects_sorted.clear();

		for (auto& [name, ptr] : this->objects)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Deleting '%s' object", name.c_str());
			delete ptr;
		}

		this->objects.clear();
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
			throw std::runtime_error("Could not sort scene, object is nullptr!");
		}

		Rendering::IRenderer* a_renderer = a.second->GetRenderer();
		Rendering::IRenderer* b_renderer = b.second->GetRenderer();

		assert(a_renderer != nullptr);
		assert(b_renderer != nullptr);

		static constexpr float positive_offset = 10.f;

		const bool is_a_ui = a_renderer->GetIsUI();
		const bool is_b_ui = b_renderer->GetIsUI();

		if (is_a_ui && is_b_ui)
		{
			// Introduce positive offset (TODO: is this necessary?)
			const float a_z = a.second->Position().z + positive_offset;
			const float b_z = b.second->Position().z + positive_offset;

			return a_z < b_z;
		}

		return std::less<std::pair<std::string, Object*>>{}(a, b);
	}

	void Scene::InternalSort(
		std::unordered_map<std::string, Object*>& from_map,
		std::vector<std::pair<std::string, Object*>>& objects
	)
	{
		std::vector<std::pair<std::string, Object*>> copy_vector;

		for (auto& iter : from_map)
		{
			assert(iter.second != nullptr);
			copy_vector.emplace_back(iter);
		}

		std::sort(copy_vector.begin(), copy_vector.end(), InternalSortCmpFunction);

		objects.clear();

		for (auto& iter : copy_vector)
		{
			objects.push_back(iter);
		}
	}

	void Scene::TriggerResort()
	{
		Scene::InternalSort(this->objects, this->objects_sorted);
	}

	void Scene::AddTemplatedObject(std::string name, std::string template_name)
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
					throw std::runtime_error("Unknown renderer type!");
				}
			}

			// Allocate Object since we already know
			// that the renderer is valid
			auto* object = new Object();

			for (auto& supply : object_template.supply_list)
			{
				ModuleMessage supply_message = {ModuleMessageType::RENDERER_SUPPLY, supply};
				with_renderer->Communicate(supply_message);
				// with_renderer->SupplyData(supply);
			}

			auto* old_renderer = object->AssignRenderer(with_renderer);
			assert(old_renderer == nullptr);

			object->UpdateHeldLogger(this->logger);
			this->AddObject(name, object);
		}
		else
		{
			throw std::runtime_error("Template with name '" + template_name + "' could not be found!");
		}
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
				Logging::LogLevel::Info,
				LOGPFX_CURRENT "Exception sorting objects! e.what(): %s",
				e.what()
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

		if (object != nullptr)
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
				Logging::LogLevel::Info,
				LOGPFX_CURRENT "Exception sorting objects! e.what(): %s",
				e.what()
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
