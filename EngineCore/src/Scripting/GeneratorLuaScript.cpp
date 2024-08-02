#include "Scripting/GeneratorLuaScript.hpp"

#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "Scripting/ILuaScript.hpp"

#include "lua.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace Engine::Scripting
{
	GeneratorLuaScript::GeneratorLuaScript(Engine* with_engine, std::string with_path)
		: ILuaScript(with_engine, std::move(with_path), false)
	{
	}

	int GeneratorLuaScript::InternalCall(const std::string& function, void* data)
	{
		auto* generator_data = static_cast<std::array<void*, 2>*>(data);

		auto* mesh		= static_cast<std::vector<Rendering::RenderingVertex>*>(generator_data->at(0));
		auto* world_pos = static_cast<glm::vec3*>(generator_data->at(1));

		lua_State* coroutine = lua_newthread(state);

		lua_getglobal(coroutine, function.c_str());

		lua_pushnumber(coroutine, static_cast<lua_Number>(world_pos->x));
		lua_pushnumber(coroutine, static_cast<lua_Number>(world_pos->y));
		lua_pushnumber(coroutine, static_cast<lua_Number>(world_pos->z));

		/*for (int i = 0; i <= lua_gettop(coroutine); i++)
		{
			// Print type of every element on stack
			printf("%u %s\n", i, lua_typename(coroutine, lua_type(coroutine, i)));
		}*/

		int last_ret = LUA_OK;
		do
		{
			last_ret = lua_resume(coroutine, 3);

			if (last_ret != LUA_YIELD)
			{
				if (last_ret != LUA_OK)
				{
					using namespace std::string_literals;

					throw std::runtime_error(
						"Exception executing generator script! Error: "s + lua_tostring(coroutine, -1)
					);
				}

				return 0;
			}

			// if (lua_istable(coroutine, -1))
			//{
			glm::vec3 vertex{};
			glm::vec3 color{};
			glm::vec2 texcoord{};

			// lua_rawgeti(coroutine, -1, 1);
			vertex.x = static_cast<float>(lua_tonumber(coroutine, -3));

			// lua_rawgeti(coroutine, -2, 2);
			vertex.y = static_cast<float>(lua_tonumber(coroutine, -2));

			// lua_rawgeti(coroutine, -3, 3);
			vertex.z = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_pop(coroutine, 3);

			lua_rawgeti(coroutine, -1, 1);
			color.r = static_cast<float>(lua_tonumber(coroutine, -1));

			lua_rawgeti(coroutine, -2, 2);
			color.g = static_cast<float>(lua_tonumber(coroutine, -1));

			lua_rawgeti(coroutine, -3, 3);
			color.b = static_cast<float>(lua_tonumber(coroutine, -1));
			lua_pop(coroutine, 4);

			// lua_rawgeti(coroutine, -7, 1);
			texcoord.x = static_cast<float>(lua_tonumber(coroutine, -2));

			// lua_rawgeti(coroutine, -8, 2);
			texcoord.y = static_cast<float>(lua_tonumber(coroutine, -1));

			lua_pop(coroutine, 2);

			mesh->emplace_back(vertex, color, texcoord);

			/* this->logger->SimpleLog(
				Logging::LogLevel::Debug3,
				"Generated vertex [i] = vertex{ %f, %f, %f }, color{ %f, %f, %f }",
				// idx,
				static_cast<double>(vertex.x),
				static_cast<double>(vertex.y),
				static_cast<double>(vertex.z),
				static_cast<double>(color.r),
				static_cast<double>(color.g),
				static_cast<double>(color.b)
			); */
			//}
			// else
			//{
			//	throw std::invalid_argument("Expected table got " + std::to_string(lua_type(state, -1)));
			//}

		} while (last_ret == LUA_YIELD);

		return 0;
	}
} // namespace Engine::Scripting
