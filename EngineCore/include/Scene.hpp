#pragma once

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "Object.hpp"
#include "PlatformSemantics.hpp"
#include "Scripting/LuaScript.hpp"

#include <map>

#include <unordered_map>

namespace Engine
{
	class Engine;

	class Scene : public InternalEngineObject
	{
	private:
	protected:
		std::vector<std::pair<std::string, Object*>> objects_sorted{};

		std::unordered_map<std::string, Object*> objects{};
		std::unordered_map<std::string, Object*> templates{};

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

		void LoadTemplatedObject(std::string name, Object* ptr);
	};
} // namespace Engine