#include "Scripting/GeneratorLuaScript.hpp"

#include "Rendering/SupplyData.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "Scripting/CrossStateFunctionCall.hpp"
#include "Scripting/ILuaScript.hpp"
#include "Scripting/Utility.hpp"

#include "lua.hpp"

#include <stdexcept>
#include <string>

namespace Engine::Scripting
{
	GeneratorLuaScript::GeneratorLuaScript(
		Engine* with_engine,
		const std::filesystem::path& with_path
	)
		: ILuaScript(with_engine, with_path, false)
	{
	}

	int GeneratorLuaScript::InternalCall(const std::string& function, void* data)
	{
		auto* generator_data = static_cast<std::array<void*, 3>*>(data);

		auto* mesh = static_cast<std::vector<Rendering::RenderingVertex>*>(generator_data->at(0));
		auto* supplier	= static_cast<Scripting::ScriptFunctionRef*>(generator_data->at(1));
		auto* world_pos = static_cast<glm::vec3*>(generator_data->at(2));

		lua_State* coroutine = lua_newthread(state);
		lua_getglobal(coroutine, function.c_str());

		if (auto locked_supplier = supplier->script.lock())
		{
			auto* state_ptr = locked_supplier->GetState();

			CreateCrossStateCallBridge(&(state_ptr), supplier->function, coroutine);
		}

		lua_pushnumber(coroutine, static_cast<lua_Number>(world_pos->x));
		lua_pushnumber(coroutine, static_cast<lua_Number>(world_pos->y));
		lua_pushnumber(coroutine, static_cast<lua_Number>(world_pos->z));

		int last_ret = LUA_OK;
		do
		{
			last_ret = lua_resume(coroutine, 4);

			if (last_ret != LUA_YIELD)
			{
				if (last_ret != LUA_OK)
				{
					using namespace std::string_literals;

					throw std::runtime_error(
						"Exception executing generator script! lua_resume error: "s
							.append(std::to_string(last_ret))
							.append(Utility::UnwindStack(coroutine))
					);
				}

				return 0;
			}

			glm::vec3 vertex{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 texcoord{};

			vertex.x = static_cast<float>(lua_tonumber(coroutine, -3));
			vertex.y = static_cast<float>(lua_tonumber(coroutine, -2));
			vertex.z = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_pop(coroutine, 3);

			// TODO: Use non-table values
			lua_rawgeti(coroutine, -1, 1);
			color.r = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_rawgeti(coroutine, -2, 2);
			color.g = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_rawgeti(coroutine, -3, 3);
			color.b = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_pop(coroutine, 4);

			// TODO: Use non-table values
			lua_rawgeti(coroutine, -1, 1);
			normal.x = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_rawgeti(coroutine, -2, 2);
			normal.y = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_rawgeti(coroutine, -3, 3);
			normal.z = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_pop(coroutine, 4);

			texcoord.x = static_cast<float>(lua_tonumber(coroutine, -2));
			texcoord.y = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_pop(coroutine, 2);

			mesh->emplace_back(vertex, color, texcoord, normal);

		} while (last_ret == LUA_YIELD);

		return 0;
	}
} // namespace Engine::Scripting
