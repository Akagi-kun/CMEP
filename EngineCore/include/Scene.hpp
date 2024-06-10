#pragma once

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "Object.hpp"
#include "PlatformSemantics.hpp"
#include "Scripting/LuaScript.hpp"

#include <map>

#include <unordered_map>

#include "Rendering/SupplyData.hpp"

namespace Engine
{
	class Engine;

	enum class RendererType {
		MIN_ENUM = 0x0000,

		TEXT = 1,
		SPRITE = 2,
		MESH = 3,

		MAX_ENUM = 0XFFFF
	};

	typedef struct ObjectTemplate_struct {
		RendererType with_renderer;
		std::vector<Rendering::RendererSupplyData> supply_list;

	} ObjectTemplate;

	class Scene : public InternalEngineObject
	{
	private:
	protected:
		// TODO: Convert all object vectors and maps to std::shared_ptr<Object>

		std::vector<std::pair<std::string, Object*>> objects_sorted{};

		std::unordered_map<std::string, Object*> objects{};
		std::unordered_map<std::string, ObjectTemplate> templates{};

		static void InternalSort(
			std::unordered_map<std::string, Object*> from_map, std::vector<std::pair<std::string, Object*>>& objects
		);

	public:
		std::multimap<EventHandling::EventType, std::pair<std::shared_ptr<Scripting::LuaScript>, std::string>>
			lua_event_handlers;

		Scene();
		~Scene();

		const std::unordered_map<std::string, Object*>* const GetAllObjects() noexcept;
		const std::vector<std::pair<std::string, Object*>>* const GetAllObjectsSorted() noexcept;

		void TriggerResort();

		void AddObject(std::string name, Object* ptr);
		Object* AddTemplatedObject(std::string name, std::string template_name);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;

		void LoadTemplatedObject(std::string name, ObjectTemplate object);
	};
} // namespace Engine