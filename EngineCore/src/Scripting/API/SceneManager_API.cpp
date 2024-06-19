#include "Scripting/API/SceneManager_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "SceneManager.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_MAPPED
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Scripting::API
{
	namespace Functions_SceneManager
	{

#pragma region SceneManager

		static int GetCameraHVRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			glm::vec2 hvrot{};
			if (auto locked_scene_manager = scene_manager.lock())
			{
				hvrot = locked_scene_manager->GetCameraHVRotation();
			}

			lua_pushnumber(state, static_cast<double>(hvrot.x));
			lua_pushnumber(state, static_cast<double>(hvrot.y));

			return 2;
		}

		static int SetCameraHVRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			double h = lua_tonumber(state, 2);
			double v = lua_tonumber(state, 3);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->SetCameraHVRotation(glm::vec2(h, v));
			}

			return 0;
		}

		static int GetCameraTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			glm::vec3 transform{};

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->GetCameraTransform();
			}

			lua_pushnumber(state, static_cast<double>(transform.x));
			lua_pushnumber(state, static_cast<double>(transform.y));
			lua_pushnumber(state, static_cast<double>(transform.z));

			return 3;
		}

		static int SetCameraTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double z = lua_tonumber(state, 4);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->SetCameraTransform(glm::vec3(x, y, z));
			}

			return 0;
		}

		static int GetLightTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			glm::vec3 transform{};
			if (auto locked_scene_manager = scene_manager.lock())
			{
				transform = locked_scene_manager->GetLightTransform();

				lua_pushnumber(state, static_cast<double>(transform.x));
				lua_pushnumber(state, static_cast<double>(transform.y));
				lua_pushnumber(state, static_cast<double>(transform.z));

				return 3;
			}
			else
			{
				return 0;
			}
		}

		static int SetLightTransform(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

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

		static int AddObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			lua_getfield(state, 3, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->AddObject(std::move(name), ptr_obj);
			}
			else
			{
				return 0;
			}

			return 0;
		}

		static int FindObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

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
						Logging::LogLevel::Warning,
						LOGPFX_CURRENT "Object %s requested but returned nullptr!",
						obj_name.c_str()
					);
				}

				lua_pushnil(state);
			}

			return 1;
		}

		static int RemoveObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->RemoveObject(std::move(name));
			}

			return 0;
		}

		static int AddTemplatedObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name		  = lua_tostring(state, 2);
			std::string template_name = lua_tostring(state, 3);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				/*Object* object = */
				locked_scene_manager->AddTemplatedObject(name, template_name);
				/*
				if (object != nullptr)
				{
					LuaObjectFactories::ObjectFactory(state, object.get());

					return 1;
				}
				else
				{
					std::weak_ptr<Logging::Logger> logger =
	API::LuaObjectFactories::MetaLoggerFactory(state);

					if (auto locked_logger = logger.lock())
					{
						locked_logger->SimpleLog(
							Logging::LogLevel::Warning,
							LOGPFX_CURRENT "Templated Object '%s' could not be added! (check if
	valid?)", name.c_str()
						);
					}
				}
				 */
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLoggerFactory(state);

				if (auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(
						Logging::LogLevel::Error,
						LOGPFX_CURRENT "Scene manager could not be locked!"
					);
				}
				// TODO: Possible lua_error?
			}

			return 0;
		}
#pragma endregion

	} // namespace Functions_SceneManager

	const std::unordered_map<std::string, lua_CFunction> scene_manager_mappings = {
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
