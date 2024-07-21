#pragma once

#include "Scripting/LuaScript.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "Object.hpp"

#include <map>
#include <unordered_map>

namespace Engine
{
	/* enum class RendererType : uint8_t
	{
		MIN_ENUM = 0x00,

		TEXT   = 1,
		SPRITE = 2,
		MESH   = 3,

		MAX_ENUM = 0XFF
	};

	struct ObjectTemplate
	{
		RendererType with_renderer;
		std::string with_shader;
		std::vector<Rendering::RendererSupplyData> supply_list;
	}; */

	class Scene : public InternalEngineObject
	{
	private:
	protected:
		std::unordered_map<std::string, Object*> objects;
		std::unordered_map<std::string, Factories::ObjectFactory::ObjectTemplate> templates;

	public:
		std::multimap<EventHandling::EventType, std::pair<std::shared_ptr<Scripting::LuaScript>, std::string>>
			lua_event_handlers;

		using InternalEngineObject::InternalEngineObject;
		~Scene();

		[[nodiscard]] const std::unordered_map<std::string, Object*>& GetAllObjects() noexcept;

		void AddObject(const std::string& name, Object* ptr);
		void AddTemplatedObject(const std::string& name, const std::string& template_name);
		[[nodiscard]] Object* FindObject(const std::string& name);
		void RemoveObject(const std::string& name);

		void LoadTemplatedObject(std::string name, Factories::ObjectFactory::ObjectTemplate object);
	};
} // namespace Engine
