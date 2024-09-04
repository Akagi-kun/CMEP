#include "Scripting/Utility.hpp"

#include "Scripting/API/AssetManager_API.hpp"
#include "Scripting/API/Engine_API.hpp"
#include "Scripting/API/Object_API.hpp"
#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/Scene_API.hpp"
#include "Scripting/Mappings.hpp"

#include "Exception.hpp"

#include <charconv>
#include <sstream>

namespace Engine::Scripting::Utility
{
	std::string UnwindStack(lua_State* of_state)
	{
		std::string error_msg =
			"\n--- BEGIN LUA STACK UNWIND ---\n\nError that caused this stack unwind:\n";

		// Cause
		std::istringstream caused_by(lua_tostring(of_state, -1));
		lua_pop(of_state, 1);

		std::string line;
		while (std::getline(caused_by, line))
		{
			error_msg.append("\t").append(line).append("\n");
		}
		error_msg.append("\n");
		// Cause

		// Stack trace
		int caller_level = 0;
		while (true)
		{
			lua_Debug activation_record = {};

			if (lua_getstack(of_state, caller_level, &activation_record) != 1)
			{
				break;
			}

			lua_getinfo(of_state, "nS", &activation_record);

			if (caller_level > 0)
			{
				error_msg.append("called by ");
			}
			else
			{
				error_msg.append("thrown by ");
			}

			error_msg.append(activation_record.source)
				.append(":")
				.append(std::to_string(activation_record.linedefined))
				.append("\n");

			caller_level++;
		}
		// Stack trace

		// caller_level will be higher than 0
		// if any stack records could be unwound
		if (caller_level == 0)
		{
			error_msg.append("could not unwind stack for this error");
		}

		error_msg.append("\n--- END LUA STACK UNWIND ---\n");

		return error_msg;
	}

	int LuaErrorHandler(lua_State* state)
	{
		// Describe error by unwinding the stack
		lua_pushstring(state, UnwindStack(state).c_str());

		// UnwindStack pops the error string
		// so we return only a single string containing the stack trace
		return 1;
	}

	void PrintStackContent(lua_State* state)
	{
		for (int i = 0; i <= lua_gettop(state); i++)
		{
			// Print type of every element on stack
			printf(
				"%u %s (%s)\n",
				i,
				lua_typename(state, lua_type(state, i)),
				lua_tostring(state, i)
			);
		}
	}

	namespace
	{
		LuaValue::Type LuaTypeGetter(lua_State* state, int stack_index)
		{
			ENGINE_EXCEPTION_ON_ASSERT(
				!lua_isnone(state, stack_index),
				"Cannot construct LuaValue from LUA_TNONE"
			)

			return static_cast<LuaValue::Type>(lua_type(state, stack_index));
		}

		LuaValue::value_t LuaValueGetter(lua_State* state, int stack_index, LuaValue::Type type)
		{
			using Type = LuaValue::Type;

			switch (type)
			{
				case Type::NUMBER:
				{
					return lua_tonumber(state, stack_index);
				}
				case Type::NIL:
				{
					return nullptr;
				}
				case Type::BOOL:
				{
					return static_cast<bool>(lua_toboolean(state, stack_index));
				}
				case Type::CDATA:
				{
					return GetCData(state, stack_index);
				}
				case Type::LUSERDATA:
				case Type::USERDATA:
				{
					return lua_touserdata(state, stack_index);
				}
				case Type::STRING:
				{
					return std::string(lua_tostring(state, stack_index));
				}
				default:
				{
					throw ENGINE_EXCEPTION("Reached default in LuaValue ctor!");
				}
			}
		}
	} // namespace

	LuaValue::LuaValue(lua_State* state, int stack_index)
		: type(LuaTypeGetter(state, stack_index)), value(LuaValueGetter(state, stack_index, type))
	{
	}

	LuaValue::operator std::string() const
	{
		ENGINE_EXCEPTION_ON_ASSERT(type == Type::STRING, )
		return std::get<std::string>(value);
	}

	LuaValue::operator double() const
	{
		ENGINE_EXCEPTION_ON_ASSERT(type == Type::NUMBER, )
		return std::get<double>(value);
	}

	LuaValue::operator void*() const
	{
		ENGINE_EXCEPTION_ON_ASSERT(
			type == Type::USERDATA || type == Type::LUSERDATA || type == Type::CDATA,
		)
		return std::get<void*>(value);
	}

	LuaValue::operator bool() const
	{
		ENGINE_EXCEPTION_ON_ASSERT(type == Type::BOOL, )
		return std::get<bool>(value);
	}

	LuaValue::operator Factories::ObjectFactory::valid_value_t() const
	{
		constexpr static auto supply_data_visitor = [](auto&& val
													) -> Factories::ObjectFactory::valid_value_t {
			using val_t = std::decay_t<decltype(val)>;
			if constexpr (std::is_same<val_t, std::string>())
			{
				return static_cast<std::string>(val);
			}
			else if constexpr (std::is_same<val_t, void*>())
			{
				return static_cast<void*>(val);
			}
			else
			{
				return {};
			}
		};

		return std::visit(supply_data_visitor, value);
	}

	LuaValue::operator uintptr_t() const
	{
		ENGINE_EXCEPTION_ON_ASSERT(
			type == Type::USERDATA || type == Type::LUSERDATA || type == Type::CDATA,
		)
		return reinterpret_cast<uintptr_t>(std::get<void*>(value));
	}

	std::string LuaValue::ToString(const LuaValue& value)
	{
		static constexpr size_t buffer_len			   = 32;
		char					ptr_buffer[buffer_len] = {};

		switch (value.type)
		{
			case Type::NUMBER:
			{
				return std::to_string(static_cast<double>(value));
			}
			case Type::NIL:
			{
				return "nil";
			}
			case Type::BOOL:
			{
				return static_cast<bool>(value) ? "true" : "false";
			}
			case Type::CDATA:
			{
				using namespace std::string_literals;

				memset(ptr_buffer, 0, buffer_len);

				std::to_chars(
					ptr_buffer,
					ptr_buffer + (buffer_len - 1),
					static_cast<uintptr_t>(value),
					16
				);

				return "cdata<? ? ?>: 0x"s.append(ptr_buffer);
			}
			case Type::LUSERDATA:
			case Type::USERDATA:
			{
				using namespace std::string_literals;

				memset(ptr_buffer, 0, buffer_len);

				std::to_chars(
					ptr_buffer,
					ptr_buffer + (buffer_len - 1),
					static_cast<uintptr_t>(value),
					16
				);

				return "userdata: 0x"s.append(ptr_buffer);
			}
			case Type::STRING:
			{
				return static_cast<std::string>(value);
			}
			default:
			{
				throw ENGINE_EXCEPTION("Reached default in LuaValue ctor!");
			}
		}
	}

	std::string_view MappingReverseLookup(lua_CFunction lookup_function)
	{
		static const std::vector<std::unordered_map<std::string, const lua_CFunction>>
			all_mappings = {
				Mappings::mappings,
				API::object_mappings,
				API::scene_manager_mappings,
				API::asset_manager_mappings,
				API::scene_mappings,
				API::engine_mappings
			};

		for (const auto& mapping : all_mappings)
		{
			for (const auto& [name, function] : mapping)
			{
				if (function == lookup_function)
				{
					return name;
				}
			}
		}

		return "[no match found]";
	}
} // namespace Engine::Scripting::Utility
