#pragma once

#include "Scripting/ILuaScript.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "Object.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Engine
{
	class Scene : public InternalEngineObject
	{
	public:
		std::multimap<
			EventHandling::EventType,
			std::pair<std::shared_ptr<Scripting::ILuaScript>, std::string>>
			lua_event_handlers;

		using InternalEngineObject::InternalEngineObject;
		~Scene();

		[[nodiscard]] const std::unordered_map<std::string, Object*>& getAllObjects() noexcept;

		void addObject(const std::string& name, Object* ptr);
		void addTemplatedObject(const std::string& name, const std::string& template_name);
		[[nodiscard]] Object* findObject(const std::string& name);
		void				  removeObject(const std::string& name);

		void loadTemplatedObject(
			const std::string&								name,
			const Factories::ObjectFactory::ObjectTemplate& object
		);

	private:
		std::unordered_map<std::string, Object*>								  objects;
		std::unordered_map<std::string, Factories::ObjectFactory::ObjectTemplate> templates;
	};
} // namespace Engine
