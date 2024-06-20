#include "Scene.hpp"

#include "Rendering/IRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"

#include "Engine.hpp"
#include "IModule.hpp"

// #include <exception>
#include <memory>
#include <stdexcept>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	Scene::~Scene()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Destructor called");

		this->templates.clear();
		this->objects_sorted.clear();

		for (auto& [name, ptr] : this->objects)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Deleting object '%s'", name.c_str());
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
		if (this->was_scene_modified)
		{
			Scene::InternalSort(this->objects, this->objects_sorted);
			this->was_scene_modified = false;
		}

		return &(this->objects_sorted);
	}

	static bool InternalSortCmpFunction(
		std::pair<std::string, Object*>& pair_a,
		std::pair<std::string, Object*>& pair_b
	)
	{
		if (pair_a.second == nullptr || pair_b.second == nullptr)
		{
			throw std::runtime_error("Could not sort scene, object is nullptr!");
		}

		Rendering::IRenderer* a_renderer = pair_a.second->GetRenderer();
		Rendering::IRenderer* b_renderer = pair_b.second->GetRenderer();

		assert(a_renderer != nullptr);
		assert(b_renderer != nullptr);

		static constexpr float positive_offset = 10.f;

		const bool is_a_ui = a_renderer->GetIsUI();
		const bool is_b_ui = b_renderer->GetIsUI();

		if (is_a_ui && is_b_ui)
		{
			// Introduce positive offset (TODO: is this necessary?)
			const float a_z = pair_a.second->Position().z + positive_offset;
			const float b_z = pair_b.second->Position().z + positive_offset;

			return a_z < b_z;
		}

		return std::less<std::pair<std::string, Object*>>{}(pair_a, pair_b);
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

	void Scene::AddTemplatedObject(const std::string& name, const std::string& template_name)
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
			auto* object	   = new Object();
			auto* old_renderer = object->AssignRenderer(with_renderer);
			assert(old_renderer == nullptr);

			for (auto& supply : object_template.supply_list)
			{
				ModuleMessage supply_message = {ModuleMessageType::RENDERER_SUPPLY, supply};
				object->ModuleBroadcast(ModuleMessageTarget::RENDERER, supply_message);
				// with_renderer->Communicate(supply_message);
			}

			// auto* old_renderer = object->AssignRenderer(with_renderer);
			// assert(old_renderer == nullptr);

			object->UpdateHeldLogger(this->logger);
			this->AddObject(name, object);
		}
		else
		{
			throw std::runtime_error("Template with name '" + template_name + "' could not be found!");
		}
	}

	void Scene::AddObject(const std::string& name, Object* ptr)
	{
		// this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Adding object \"%s\"", name.c_str());
		if (ptr != nullptr)
		{
			Rendering::GLFWwindowData data = this->owner_engine->GetRenderingEngine()->GetWindow();
			ptr->ScreenSizeInform(data.window_x, data.window_y);
			ptr->UpdateHeldLogger(this->logger);
			this->objects.emplace(name, ptr);
		}
		else
		{
			throw std::runtime_error("Cannot add object'" + name + "', it is nullptr!");
		}

		this->was_scene_modified = true;
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
			// this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Removing object '%s'", name.c_str());
			this->objects.erase(name);
			delete object;
		}
		else
		{
			throw std::runtime_error("Cannot add object'" + name + "', object is nullptr!");
		}

		this->was_scene_modified = true;
	}

	void Scene::LoadTemplatedObject(std::string name, ObjectTemplate object)
	{
		this->templates.emplace(name, object);
	}
} // namespace Engine
