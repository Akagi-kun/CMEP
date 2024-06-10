#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/LuaFactories.hpp"

namespace Engine::Scripting::API
{
	namespace Functions_SceneManager
	{

#pragma region SceneManager

		int GetCameraHVRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			glm::vec2 hvrot{};
			if (auto locked_scene_manager = scene_manager.lock())
			{
				hvrot = locked_scene_manager->GetCameraHVRotation();
			}

			lua_pushnumber(state, hvrot.x);
			lua_pushnumber(state, hvrot.y);

			return 2;
		}

		int SetCameraHVRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double h = lua_tonumber(state, 2);
			double v = lua_tonumber(state, 3);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->SetCameraHVRotation(glm::vec2(h, v));
			}

			return 0;
		}

		int GetCameraTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			glm::vec3 transform{};

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->GetCameraTransform();
			}

			lua_pushnumber(state, transform.x);
			lua_pushnumber(state, transform.y);
			lua_pushnumber(state, transform.z);

			return 3;
		}

		int SetCameraTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double z = lua_tonumber(state, 4);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->SetCameraTransform(glm::vec3(x, y, z));
			}

			return 0;
		}

		int GetLightTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			glm::vec3 transform{};
			if (auto locked_scene_manager = scene_manager.lock())
			{
				transform = locked_scene_manager->GetLightTransform();

				lua_pushnumber(state, transform.x);
				lua_pushnumber(state, transform.y);
				lua_pushnumber(state, transform.z);

				return 3;
			}
			else
			{
				return 0;
			}
		}

		int SetLightTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double z = lua_tonumber(state, 4);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->SetLightTransform(glm::vec3(x, y, z));
			}
			else
			{
				return 0;
			}

			return 0;
		}

		int AddObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);

			lua_getfield(state, 3, "_pointer");
			Object* obj = *(Object**)lua_touserdata(state, -1);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->AddObject(std::move(name), obj);
			}
			else
			{
				return 0;
			}

			return 0;
		}

		int FindObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			std::string obj_name = lua_tostring(state, 2);

			Object* obj;
			if (auto locked_scene_manager = scene_manager.lock())
			{
				obj = locked_scene_manager->FindObject(obj_name);
			}
			else
			{
				return 0;
			}

			if (obj != nullptr)
			{
				API::LuaObjectFactories::ObjectFactory(state, obj);
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLoggerFactory(state);

				if (auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(
						Logging::LogLevel::Warning, "Lua: Object %s requested but returned nullptr!", obj_name.c_str()
					);
				}

				lua_pushnil(state);
			}

			return 1;
		}

		int RemoveObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->RemoveObject(std::move(name));
			}

			return 0;
		}

		int AddTemplatedObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);

			std::string template_name = lua_tostring(state, 3);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				Object* object = locked_scene_manager->AddTemplatedObject(std::move(name), std::move(template_name));

				if (object != nullptr)
				{
					LuaObjectFactories::ObjectFactory(state, object);

					return 1;
				}
				else
				{
					std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLoggerFactory(state);

					if (auto locked_logger = logger.lock())
					{
						locked_logger->SimpleLog(
							Logging::LogLevel::Warning,
							"Lua: Templated Object %s could not be added ! (check if valid?)",
							name.c_str()
						);
					}
				}
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLoggerFactory(state);

				if (auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Error, "Lua: Scene manager could not be locked!");
				}
				// TODO: Possible lua_error?
			}

			return 0;
		}
#pragma endregion

	} // namespace Functions_SceneManager

	std::unordered_map<std::string, lua_CFunction> scene_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetCameraHVRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetCameraHVRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetCameraTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetCameraTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetLightTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetLightTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, AddObject),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, FindObject),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, RemoveObject),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, AddTemplatedObject)
	};
} // namespace Engine::Scripting::API