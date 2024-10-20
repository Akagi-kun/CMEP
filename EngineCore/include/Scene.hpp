#pragma once

#include "Assets/AssetManager.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "SceneObject.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	class Scene : public InternalEngineObject
	{
	public:
		std::multimap<EventHandling::EventType, Scripting::ScriptFunctionRef> lua_event_handlers;

		std::unique_ptr<AssetRepository> asset_repository;

		Scene(Engine* with_engine);
		~Scene();

		[[nodiscard]] const std::unordered_map<std::string, SceneObject*>& getAllObjects() noexcept;

		void addObject(const std::string& name, SceneObject* ptr);
		void addTemplatedObject(const std::string& name, const std::string& template_name);
		[[nodiscard]] SceneObject* findObject(const std::string& name);
		void					   removeObject(const std::string& name);

		void loadTemplatedObject(
			const std::string&								name,
			const Factories::ObjectFactory::ObjectTemplate& object
		);

	private:
		std::unordered_map<std::string, SceneObject*>							  objects;
		std::unordered_map<std::string, Factories::ObjectFactory::ObjectTemplate> templates;
	};
} // namespace Engine
