#include "Scripting/API/Global_API.hpp"

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/SupplyData.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"
#include "Scripting/ILuaScript.hpp"
#include "Scripting/LuaValue.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "EnumStringConvertor.hpp"
#include "Exception.hpp"
#include "lua.hpp"

#include <cassert>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::Scripting::API
{
	int printReplace(lua_State* state)
	{
		int argc = lua_gettop(state);

		std::weak_ptr<Logging::Logger> logger = *static_cast<std::weak_ptr<Logging::Logger>*>(
			lua_touserdata(state, lua_upvalueindex(1))
		);

		auto locked_logger = CHECK(logger);

		locked_logger->startLog<void>(Logging::LogLevel::Info);

		for (int arg = 1; arg <= argc; arg++)
		{
			LuaValue	value(state, arg);
			std::string string = LuaValue::toString(value);

			locked_logger->log("{}\t", string.c_str());
		}

		locked_logger->stopLog();

		return 0;
	}

	/// @cond LUA_API
	namespace
	{
#pragma region ObjectFactory

		using namespace Factories::ObjectFactory;

		ScriptFunctionRef valueToScriptFnRef(LuaValue source)
		{
			return ScriptFunctionRef{
				.script	  = *static_cast<std::weak_ptr<Scripting::ILuaScript>*>(UNWRAP(source[1])),
				.function = UNWRAP(source[2])
			};
		}

		template <typename supply_data_t>
		[[nodiscard]] supply_data_t interpretSupplyData(LuaValue table)
		{
			assert(table.toTable().size() == 2);

			EnumStringConvertor<typename supply_data_t::Type> type	= UNWRAP(table[1]);
			LuaValue										  value = UNWRAP(table[2]);

			if constexpr (std::is_same_v<supply_data_t, Rendering::RendererSupplyData>)
			{
				return generateRendererSupplyData(type, static_cast<supply_data_value_t>(value));
			}
			else if constexpr (std::is_same_v<supply_data_t, Rendering::MeshBuilderSupplyData>)
			{
				if (type == Rendering::MeshBuilderSupplyData::Type::GENERATOR)
				{
					Rendering::GeneratorData gen_data = {
						.generator = valueToScriptFnRef(UNWRAP(value[1])),
						.supplier  = valueToScriptFnRef(UNWRAP(value[2])),
					};

					return {type, gen_data};
				}

				return generateMeshBuilderSupplyData(type, static_cast<supply_data_value_t>(value));
			}
		} // namespace

		template <typename supply_data_t>
		std::vector<supply_data_t> interpretSupplyDataTable(const LuaValue& table)
		{
			std::vector<supply_data_t> supply_data;

			EXCEPTION_ASSERT(
				table.type == LuaValue::Type::TABLE,
				"Tried passing non-table value as a supply data table!"
			);

			for (const auto& [key, val] : table.toTable())
			{
				try
				{
					supply_data.emplace_back(interpretSupplyData<supply_data_t>(val));
				}
				catch (...)
				{
					std::throw_with_nested(
						ENGINE_EXCEPTION("Exception occured during interpreting of supply data")
					);
				}
			}

			return supply_data;
		}

		int createSceneObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 6)

			auto* owner_engine = getObjectAsPointer<Engine>(state, 1);

			std::string renderer_type	  = LuaValue(state, 2);
			std::string mesh_builder_type = LuaValue(state, 3);

			std::string shader_name = LuaValue(state, 4);

			// Parse renderer supply data
			std::vector<Rendering::RendererSupplyData> renderer_supply_data =
				interpretSupplyDataTable<Rendering::RendererSupplyData>(LuaValue(state, 5));

			// Parse mesh builder supply data
			std::vector<Rendering::MeshBuilderSupplyData> meshbuilder_supply_data =
				interpretSupplyDataTable<Rendering::MeshBuilderSupplyData>(LuaValue(state, 6));

			// Get a factory for the desired object
			const auto factory = getSceneObjectFactory(renderer_type, mesh_builder_type);

			if (!factory)
			{
				return luaL_error(state, "No factory was found for this object!");
			}

			// Invoke the factory
			SceneObject* obj =
				factory(owner_engine, shader_name, renderer_supply_data, meshbuilder_supply_data);

			if (obj == nullptr)
			{
				return luaL_error(state, "Object was nullptr!");
			}

			API::LuaFactories::templatedFactory<SceneObject>(state, obj);

			return 1;
		}

#pragma endregion

#pragma region Renderer / MeshBuilder

		int rendererSupplyData(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			auto* self = getObjectAsPointer<Rendering::IRenderer>(state, 1);

			auto type  = LuaValue(state, 2);
			auto value = LuaValue(state, 3);

			self->supplyData(
				generateRendererSupplyData(type, static_cast<supply_data_value_t>(value))
			);

			return 0;
		}

		int meshBuilderSupplyData(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			auto* self = getObjectAsPointer<Rendering::IMeshBuilder>(state, 1);

			auto type  = LuaValue(state, 2);
			auto value = LuaValue(state, 3);

			self->supplyData(
				generateMeshBuilderSupplyData(type, static_cast<supply_data_value_t>(value))
			);

			return 0;
		}

#pragma endregion

	} // namespace
	/// @endcond

	std::unordered_map<std::string, const lua_CFunction> global_mappings = {
		CMEP_LUAMAPPING_DEFINE(rendererSupplyData),
		CMEP_LUAMAPPING_DEFINE(meshBuilderSupplyData),

		CMEP_LUAMAPPING_DEFINE(createSceneObject)
	};
} // namespace Engine::Scripting::API
