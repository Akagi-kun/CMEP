#include "Scripting/GeneratorLuaScript.hpp"

#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "lua.h"

#include <stdexcept>
#include <string>

namespace Engine::Scripting
{
	int GeneratorLuaScript::InternalCall(const std::string& function, void* data)
	{
		auto* mesh = static_cast<std::vector<Rendering::RenderingVertex>*>(data);

		uint_fast32_t idx = 0;
		bool is_done	  = false;
		do
		{
			this->InitializeCall(function);

			// Push index and call lua
			lua_pushinteger(state, idx);

			// TODO: This should yield instead of returning every value
			int return_value = lua_pcall(state, 1, 2, -3);
			if (return_value != LUA_OK)
			{
				throw std::runtime_error(
					"Calling generator resulted in non-zero value " + std::to_string(return_value)
				);
			}

			is_done = (lua_toboolean(state, -2) != 0);

			if (is_done)
			{
				break;
			}

			if (lua_istable(state, -1))
			{
				glm::vec3 vertex{};

				lua_rawgeti(state, -1, 1);
				vertex.x = static_cast<float>(lua_tonumber(state, -1));

				lua_rawgeti(state, -2, 2);
				vertex.y = static_cast<float>(lua_tonumber(state, -1));

				lua_rawgeti(state, -3, 3);
				vertex.z = static_cast<float>(lua_tonumber(state, -1));

				lua_pop(state, 4);

				mesh->push_back(Rendering::RenderingVertex{vertex, glm::vec3(1.0, 1.0, 0.0)});

				this->logger->SimpleLog(
					Logging::LogLevel::Debug3,
					"Generated vertex [%u] = { %f, %f, %f }",
					idx,
					static_cast<double>(vertex.x),
					static_cast<double>(vertex.y),
					static_cast<double>(vertex.z)
				);
			}
			else
			{
				throw std::invalid_argument("Expected table got " + std::to_string(lua_type(state, -1)));
			}

			idx++;
		} while (idx < 128);

		return 0;
	}
} // namespace Engine::Scripting
