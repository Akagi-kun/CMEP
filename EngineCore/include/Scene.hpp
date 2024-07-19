#pragma once

#include "Rendering/SupplyData.hpp"

#include "Scripting/LuaScript.hpp"

#include "EventHandling.hpp"
#include "InternalEngineObject.hpp"
#include "Object.hpp"

#include <map>
#include <unordered_map>

namespace Engine
{
	class Engine;

	enum class RendererType : uint8_t
	{
		MIN_ENUM = 0x00,

		TEXT   = 1,
		SPRITE = 2,
		MESH   = 3,

		MAX_ENUM = 0XFF
	};

	typedef struct ObjectTemplate_struct
	{
		RendererType with_renderer;
		std::vector<Rendering::RendererSupplyData> supply_list;

	} ObjectTemplate;

	class Scene : public InternalEngineObject
	{
	private:
	protected:
		std::vector<Object*> objects_sorted;

		std::unordered_map<std::string, Object*> objects;
		std::unordered_map<std::string, ObjectTemplate> templates;

		static void InternalSort(
			const std::unordered_map<std::string, Object*>& from_map,
			std::vector<Object*>& to_vector
		);

	public:
		bool was_scene_modified = false;

		std::multimap<EventHandling::EventType, std::pair<std::shared_ptr<Scripting::LuaScript>, std::string>>
			lua_event_handlers;

		using InternalEngineObject::InternalEngineObject;
		// Scene() = default;
		~Scene();

		[[nodiscard]] const std::unordered_map<std::string, Object*>* GetAllObjects() noexcept;
		[[nodiscard]] const std::vector<Object*>& GetAllObjectsSorted() noexcept;

		void AddObject(const std::string& name, Object* ptr);
		void AddTemplatedObject(const std::string& name, const std::string& template_name);
		[[nodiscard]] Object* FindObject(const std::string& name);
		void RemoveObject(const std::string& name);

		void LoadTemplatedObject(std::string name, ObjectTemplate object);
	};
} // namespace Engine
