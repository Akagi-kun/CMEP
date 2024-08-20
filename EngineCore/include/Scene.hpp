#pragma once

#include "Scripting/ILuaScript.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "Object.hpp"

#include <map>
#include <unordered_map>

namespace Engine
{
	class Scene : public InternalEngineObject
	{
	private:
		std::unordered_map<std::string, Object*> objects;
		std::unordered_map<std::string, Factories::ObjectFactory::ObjectTemplate> templates;

	public:
		std::multimap<EventHandling::EventType, std::pair<std::shared_ptr<Scripting::ILuaScript>, std::string>>
			lua_event_handlers;

		using InternalEngineObject::InternalEngineObject;
		~Scene();

		[[nodiscard]] const std::unordered_map<std::string, Object*>& GetAllObjects() noexcept;

		void AddObject(const std::string& name, Object* ptr);
		void AddTemplatedObject(const std::string& name, const std::string& template_name);
		[[nodiscard]] Object* FindObject(const std::string& name);
		void RemoveObject(const std::string& name);

		void LoadTemplatedObject(const std::string& name, const Factories::ObjectFactory::ObjectTemplate& object);
	};
} // namespace Engine
