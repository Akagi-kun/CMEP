#include "glm/glm.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Scripting/Mappings.hpp"
#include "Rendering/Texture.hpp"

#include "GlobalSceneManager.hpp"
#include "AssetManager.hpp"
#include "ObjectFactory.hpp"
#include "Engine.hpp"

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
#pragma region GlobalSceneManager

		int gsm_GetCameraHVRotation(lua_State* state)
		{
			glm::vec2 hvrot = global_scene_manager->GetCameraHVRotation();

			lua_pushnumber(state, hvrot.x);
			lua_pushnumber(state, hvrot.y);

			return 2;
		}

		int gsm_SetCameraHVRotation(lua_State* state)
		{
			double h = lua_tonumber(state, 1);
			double v = lua_tonumber(state, 2);

			global_scene_manager->SetCameraHVRotation(glm::vec2(h, v));

			return 0;
		}

		int gsm_GetCameraTransform(lua_State* state)
		{
			glm::vec3 transform = global_scene_manager->GetCameraTransform();

			lua_pushnumber(state, transform.x);
			lua_pushnumber(state, transform.y);
			lua_pushnumber(state, transform.z);

			return 3;
		}

		int gsm_SetCameraTransform(lua_State* state)
		{
			double x = lua_tonumber(state, 1);
			double y = lua_tonumber(state, 2);
			double z = lua_tonumber(state, 3);

			global_scene_manager->SetCameraTransform(glm::vec3(x, y, z));

			return 0;
		}

		int gsm_GetLightTransform(lua_State* state)
		{
			glm::vec3 transform = global_scene_manager->GetLightTransform();

			lua_pushnumber(state, transform.x);
			lua_pushnumber(state, transform.y);
			lua_pushnumber(state, transform.z);

			return 3;
		}


		int gsm_SetLightTransform(lua_State* state)
		{
			double x = lua_tonumber(state, 1);
			double y = lua_tonumber(state, 2);
			double z = lua_tonumber(state, 3);

			global_scene_manager->SetLightTransform(glm::vec3(x, y, z));

			return 0;
		}

		int gsm_AddObject(lua_State* state)
		{
			std::string name = lua_tostring(state, 1);
			
			lua_getfield(state, 2, "_pointer");
			Object* obj = *(Object**)lua_touserdata(state, -1);

			global_scene_manager->AddObject(name, obj);

			return 0;
		}

		int gsm_FindObject(lua_State* state)
		{
			std::string obj_name = lua_tostring(state, 1);
			lua_pop(state, 1);

			Object* obj = global_scene_manager->FindObject(obj_name);

			if (obj != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object %s requested but returned nullptr!", obj_name.c_str());

				lua_pushnil(state);
			}

			return 1;
		}

		int gsm_RemoveObject(lua_State* state)
		{
			std::string name = lua_tostring(state, 1);

			global_scene_manager->RemoveObject(name);

			return 0;
		}
#pragma endregion

#pragma region Engine

		int engine_GetAssetManager(lua_State* state)
		{
			AssetManager* asset_manager = global_engine->GetAssetManager();

			if (asset_manager != nullptr)
			{
				// Generate asset_manager table
				lua_newtable(state);

				AssetManager** ptr_obj = (AssetManager**)lua_newuserdata(state, sizeof(AssetManager*));
				(*ptr_obj) = asset_manager;
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Lua: AssetManager requested but returned nullptr!");

				lua_pushnil(state);
			}

			return 1;
		}

		int engine_SetFramerateTarget(lua_State* state)
		{
			unsigned int framerate_target = static_cast<unsigned int>(lua_tointeger(state, 1));

			global_engine->SetFramerateTarget(framerate_target);

			return 0;
		}

#pragma endregion

#pragma region TextRenderer

		int textRenderer_UpdateText(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Rendering::IRenderer* renderer = *(Rendering::IRenderer**)lua_touserdata(state, -1);

			const char* newtext = lua_tostring(state, 2);

			((::Engine::Rendering::TextRenderer*)renderer)->UpdateText(std::string(newtext));

			return 0;
		}

#pragma endregion

#pragma region MeshRenderer

		int meshRenderer_UpdateTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Rendering::IRenderer* renderer = *(Rendering::IRenderer**)lua_touserdata(state, -1);

			lua_getfield(state, 2, "_pointer");
			Rendering::Texture* texture = *(Rendering::Texture**)lua_touserdata(state, -1);

			((::Engine::Rendering::MeshRenderer*)renderer)->UpdateTexture(texture);

			return 0;
		}

#pragma endregion

#pragma region Object

		int object_AddChild(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			
			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			lua_getfield(state, 2, "_pointer");

			Object* ptr_child = *(Object**)lua_touserdata(state, -1);
			
			ptr_obj->AddChild(ptr_child);
			
			return 0;
		}

		int object_GetRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 rotation = ptr_obj->rotation();

			lua_pushnumber(state, rotation.x);
			lua_pushnumber(state, rotation.y);
			lua_pushnumber(state, rotation.z);

			return 3;
		}

		int object_Rotate(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");

			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 rotation = glm::vec3(0);
			rotation.x = static_cast<float>(lua_tonumber(state, 2));
			rotation.y = static_cast<float>(lua_tonumber(state, 3));
			rotation.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Rotate(rotation);

			return 0;
		}

		int object_GetPosition(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 rotation = ptr_obj->position();

			lua_pushnumber(state, rotation.x);
			lua_pushnumber(state, rotation.y);
			lua_pushnumber(state, rotation.z);

			return 3;
		}

		int object_Translate(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");

			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 position = glm::vec3(0);
			position.x = static_cast<float>(lua_tonumber(state, 2));
			position.y = static_cast<float>(lua_tonumber(state, 3));
			position.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Translate(position);

			return 0;
		}

#pragma endregion

#pragma region AssetManager

		int assetManager_GetFont(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			Rendering::Font* font = ptr_am->GetFont(path);

			if (font != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Rendering::Font** ptr = (Rendering::Font**)lua_newuserdata(state, sizeof(Rendering::Font*));
				(*ptr) = font;
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				lua_pushnil(state);
			}

			return 1;
		}

		int assetManager_GetTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			Rendering::Texture* texture = ptr_am->GetTexture(path);

			if (texture != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Rendering::Texture** ptr = (Rendering::Texture**)lua_newuserdata(state, sizeof(Rendering::Texture*));
				(*ptr) = texture;
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				lua_pushnil(state);
			}

			return 1;
		}

		int assetManager_AddTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);
			std::string path = lua_tostring(state, 3);

			ptr_am->AddTexture(name, path, Rendering::Texture_InitFiletype::FILE_PNG);

			return 1;
		}

#pragma endregion

#pragma region ObjectFactory

		int objectFactory_CreateSpriteObject(lua_State* state)
		{
			double x = lua_tonumber(state, 1);
			double y = lua_tonumber(state, 2);
			double sizex = lua_tonumber(state, 3);
			double sizey = lua_tonumber(state, 4);

			lua_getfield(state, 5, "_pointer");
			Rendering::Texture* sprite = *(Rendering::Texture**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateSpriteObject(x, y, sizex, sizey, sprite);

			if (obj != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object creation failed, ObjectFactory::CreateSpriteObject returned nullptr! Params: %f %f %f %f", x, y, sizex, sizey);

				lua_pushnil(state);
			}

			return 1;
		}

		int objectFactory_CreateTextObject(lua_State* state)
		{
			double x = lua_tonumber(state, 1);
			double y = lua_tonumber(state, 2);
			int size = static_cast<int>(lua_tointeger(state, 3));

			std::string text = lua_tostring(state, 4);

			lua_getfield(state, 5, "_pointer");
			Rendering::Font* font = *(Rendering::Font**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateTextObject(x, y, size, text, font);

			if (obj != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object creation failed, ObjectFactory::CreateTextObject returned nullptr! Params: %f %f %u '%s'", x, y, size, text.c_str());

				lua_pushnil(state);
			}

			return 1;
		}

		int objectFactory_CreateGeneric3DObject(lua_State* state)
		{
			double x = lua_tonumber(state, 1);
			double y = lua_tonumber(state, 2);
			double z = lua_tonumber(state, 3);
			double xsize = lua_tonumber(state, 4);
			double ysize = lua_tonumber(state, 5);
			double zsize = lua_tonumber(state, 6);
			double xrot = lua_tonumber(state, 7);
			double yrot = lua_tonumber(state, 8);
			double zrot = lua_tonumber(state, 9);

			lua_getfield(state, 10, "_self");
			Rendering::Mesh* mesh = *(Rendering::Mesh**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateGeneric3DObject(x, y, z, xsize, ysize, zsize, xrot, yrot, zrot, mesh);

			if (obj != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				lua_pushnil(state);
			}

			return 1;
		}
#pragma endregion
	
#pragma region Mesh

		int mesh_Mesh(lua_State* state)
		{
			Rendering::Mesh* mesh = new Rendering::Mesh();

			// Generate table
			lua_newtable(state);

			Rendering::Mesh** ptr_mesh = (Rendering::Mesh**)lua_newuserdata(state, sizeof(Rendering::Mesh*));
			(*ptr_mesh) = mesh;
			lua_setfield(state, -2, "_self");

			return 1;
		}

		int mesh_CreateMeshFromObj(lua_State* state)
		{
			lua_getfield(state, 1, "_self");
			Rendering::Mesh** mesh = (Rendering::Mesh**)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			(*mesh)->CreateMeshFromObj(path);

			return 0;
		}

#pragma endregion
	}

	const char* nameMappings[] = {
		"gsm_GetCameraHVRotation",
		"gsm_SetCameraHVRotation",
		"gsm_GetCameraTransform",
		"gsm_SetCameraTransform",
		"gsm_GetLightTransform",
		"gsm_SetLightTransform",
		"gsm_AddObject",
		"gsm_FindObject",
		"gsm_RemoveObject",

		"engine_GetAssetManager",
		"engine_SetFramerateTarget",

		"textRenderer_UpdateText",

		"meshRenderer_UpdateTexture",

		"object_AddChild",
		"object_GetRotation",
		"object_Rotate",
		"object_GetPosition",
		"object_Translate",

		"assetManager_GetFont",
		"assetManager_GetTexture",
		"assetManager_AddTexture",

		"objectFactory_CreateSpriteObject",
		"objectFactory_CreateTextObject",
		"objectFactory_CreateGeneric3DObject",

		"mesh_Mesh",
		"mesh_CreateMeshFromObj"
	};

	lua_CFunction functionMappings[] = {
		Functions::gsm_GetCameraHVRotation,
		Functions::gsm_SetCameraHVRotation,
		Functions::gsm_GetCameraTransform,
		Functions::gsm_SetCameraTransform,
		Functions::gsm_GetLightTransform,
		Functions::gsm_SetLightTransform,
		Functions::gsm_AddObject,
		Functions::gsm_FindObject,
		Functions::gsm_RemoveObject,

		Functions::engine_GetAssetManager,
		Functions::engine_SetFramerateTarget,

		Functions::textRenderer_UpdateText,

		Functions::meshRenderer_UpdateTexture,

		Functions::object_AddChild,
		Functions::object_GetRotation,
		Functions::object_Rotate,
		Functions::object_GetPosition,
		Functions::object_Translate,

		Functions::assetManager_GetFont,
		Functions::assetManager_GetTexture,
		Functions::assetManager_AddTexture,

		Functions::objectFactory_CreateSpriteObject,
		Functions::objectFactory_CreateTextObject,
		Functions::objectFactory_CreateGeneric3DObject,

		Functions::mesh_Mesh,
		Functions::mesh_CreateMeshFromObj
	};
}