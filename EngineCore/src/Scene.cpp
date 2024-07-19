#include "Scene.hpp"

#include "Rendering/IRenderer.hpp"
#include "Rendering/Renderer2D.hpp"
#include "Rendering/SpriteMeshBuilder.hpp"
#include "Rendering/TextMeshBuilder.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <iterator>
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

	const std::vector<Object*>& Scene::GetAllObjectsSorted() noexcept
	{
		if (this->was_scene_modified)
		{
			Scene::InternalSort(this->objects, this->objects_sorted);
			this->was_scene_modified = false;
		}

		return this->objects_sorted;
	}

	static bool InternalSortCmpFunction(Object*& object_a, Object*& object_b)
	{
		if (object_a == nullptr || object_b == nullptr)
		{
			throw std::runtime_error("Could not sort scene, object is nullptr!");
		}

		auto* a_renderer = static_cast<Rendering::IRenderer*>(object_a->GetRenderer());
		auto* b_renderer = static_cast<Rendering::IRenderer*>(object_b->GetRenderer());

		assert(a_renderer != nullptr);
		assert(b_renderer != nullptr);

		static constexpr float positive_offset = 10.f;

		const bool is_a_ui = a_renderer->GetIsUI();
		const bool is_b_ui = b_renderer->GetIsUI();

		if (is_a_ui && is_b_ui)
		{
			// Introduce positive offset (TODO: is this necessary?)
			const float a_z = object_a->GetPosition().z + positive_offset;
			const float b_z = object_b->GetPosition().z + positive_offset;

			return std::less<float>{}(a_z, b_z);
		}

		return std::less<Object*>{}(object_a, object_b);
	}

	void Scene::InternalSort(const std::unordered_map<std::string, Object*>& from_map, std::vector<Object*>& to_vector)
	{
		std::vector<Object*> buffer_vector;
		buffer_vector.reserve(from_map.size());

		for (const auto& iter : from_map)
		{
			assert(iter.second != nullptr);
			buffer_vector.emplace_back(iter.second);
		}

		std::sort(buffer_vector.begin(), buffer_vector.end(), InternalSortCmpFunction);

		to_vector.clear();
		to_vector.reserve(from_map.size());

		std::copy(buffer_vector.begin(), buffer_vector.end(), std::back_inserter(to_vector));

		/* for (auto& iter : buffer_vector)
		{
			to_vector.push_back(iter);
		} */
	}

	void Scene::AddTemplatedObject(const std::string& name, const std::string& template_name)
	{
		auto templated_object = this->templates.find(template_name);

		if (templated_object != this->templates.end())
		{
			ObjectTemplate object_template = templated_object->second;

			Object* obj = nullptr;

			switch (object_template.with_renderer)
			{
				case RendererType::SPRITE:
				{
					obj = ObjectFactory::CreateSceneObject<Rendering::Renderer2D, Rendering::SpriteMeshBuilder>(
						this->GetOwnerEngine(),
						object_template.supply_list,
						"sprite"
					);

					break;
				}
				case RendererType::TEXT:
				{
					obj = ObjectFactory::CreateSceneObject<Rendering::Renderer2D, Rendering::TextMeshBuilder>(
						this->GetOwnerEngine(),
						object_template.supply_list,
						"text"
					);

					break;
				}
				default:
				{
					// Unknown renderer type, cannot add object
					throw std::runtime_error("Unknown renderer type!");
				}
			}

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

			Rendering::GLFWwindowData data = this->owner_engine->GetRenderingEngine()->GetWindow();
			ptr->ScreenSizeInform(data.window_x, data.window_y);

			this->objects.emplace(name, ptr);
			this->was_scene_modified = true;
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

			this->was_scene_modified = true;
		}
		else
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Warning,
				LOGPFX_CURRENT "Tried to remove scene object '%s'! No such object exists!",
				name.c_str()
			);
		}
	}

	void Scene::LoadTemplatedObject(std::string name, ObjectTemplate object)
	{
		this->templates.emplace(name, object);
	}
} // namespace Engine
