#include "Scripting/LuaValue.hpp"

#include "C:/Users/sla-t/source/repos/CMEP/EngineCore/include/Exception.hpp"
#include "C:/Users/sla-t/source/repos/CMEP/EngineCore/include/Factories/ObjectFactory.hpp"
#include "C:/Users/sla-t/source/repos/CMEP/EngineCore/include/Scripting/Utility.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <type_traits>
#include <variant>

namespace Engine::Scripting
{
	namespace
	{
		LuaValue::Type luaTypeGetter(lua_State* state, int stack_index)
		{
			ENGINE_EXCEPTION_ON_ASSERT(
				!lua_isnone(state, stack_index),
				"Cannot construct LuaValue from LUA_TNONE"
			)

			return static_cast<LuaValue::Type>(lua_type(state, stack_index));
		}

		// Ensure type is Type::TABLE when calling
		// recursive if table contains another table
		LuaValue::value_t luaTableParser(lua_State* state, int stack_index)
		{
			LuaValue::table_t table;
			auto abs_index = Utility::relativeToAbsoluteIndex(state, stack_index);

			/* printf(
				"\nStart table parse... \t%u %s\n",
				lua_gettop(state),
				lua_typename(state, lua_type(state, -1))
			); */

			// Push nil as the first key (iterates whole table)
			lua_pushnil(state);
			while (lua_next(state, abs_index) != 0)
			{
				auto key   = LuaValue(state, -2);
				auto value = LuaValue(state, -1);

				// Keys can be any value except nil per PIL 2.5 - Tables
				ENGINE_EXCEPTION_ON_ASSERT(
					key.type != LuaValue::Type::NIL,
					"PIL 2.5 - Tables states that 'nil' is not a valid key!"
				)

				/* printf(
					"key: %s \t\t%u %s\n",
					LuaValue::toString(key).c_str(),
					lua_gettop(state),
					lua_typename(state, lua_type(state, -1))
				); */

				table.emplace_back(key, value);

				// Pop value, keep key on stack!
				lua_pop(state, 1);
			}
			/* printf(
				"End table parse \t%u %s\n\n",
				lua_gettop(state),
				lua_typename(state, lua_type(state, -1))
			); */

			// Pops the last key
			// lua_pop(state, 1);

			return table;
		}

		LuaValue::value_t luaValueGetter(lua_State* state, int stack_index, LuaValue::Type type)
		{
			using Type = LuaValue::Type;

			switch (type)
			{
				case Type::NUMBER:	  return lua_tonumber(state, stack_index);

				case Type::NIL:		  return nullptr;

				case Type::TABLE:	  return luaTableParser(state, stack_index);

				case Type::CDATA:	  return Utility::getCData(state, stack_index);

				case Type::LUSERDATA:
				case Type::USERDATA:  return lua_touserdata(state, stack_index);

				case Type::STRING:	  return std::string(lua_tostring(state, stack_index));

				case Type::BOOL:
					return static_cast<bool>(lua_toboolean(state, stack_index));

				default:
				{
					throw ENGINE_EXCEPTION("Reached default in LuaValue ctor!");
				}
			}
		}
	} // namespace

	LuaValue::LuaValue(std::nullptr_t) : type(Type::NIL), value(nullptr)
	{}
	LuaValue::LuaValue(string_t val) : type(Type::STRING), value(val)
	{}
	LuaValue::LuaValue(number_t val) : type(Type::NUMBER), value(val)
	{}

	LuaValue::LuaValue(lua_State* state, int stack_index)
		: type(luaTypeGetter(state, stack_index)),
		  value(luaValueGetter(state, stack_index, type))
	{}

	// Base getters
	LuaValue::operator string_t() const
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(type == Type::STRING)
		return std::get<string_t>(value);
	}

	LuaValue::operator number_t() const
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(type == Type::NUMBER)
		return std::get<number_t>(value);
	}

	LuaValue::operator ptr_t() const
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(
			type == Type::USERDATA || type == Type::LUSERDATA || type == Type::CDATA
		)
		return std::get<ptr_t>(value);
	}

	LuaValue::operator bool() const
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(type == Type::BOOL)
		return std::get<bool>(value);
	}

	// Specialized getters
	LuaValue::operator Factories::ObjectFactory::supply_data_value_t() const
	{
		// Utility conversion to supply data variant
		constexpr static auto supply_data_visitor =
			[](auto&& val) -> Factories::ObjectFactory::supply_data_value_t {
			using val_t = std::decay_t<decltype(val)>;
			// Handle valid types
			if constexpr (std::is_same_v<val_t, std::string>)
			{
				return static_cast<std::string>(val);
			}
			else if constexpr (std::is_same_v<val_t, void*>)
			{
				return static_cast<void*>(val);
			}
			else { return {}; }
		};

		return std::visit(supply_data_visitor, value);
	}

	LuaValue::operator uintptr_t() const
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(
			type == Type::USERDATA || type == Type::LUSERDATA || type == Type::CDATA
		)
		return reinterpret_cast<uintptr_t>(std::get<void*>(value));
	}

	namespace
	{
		template <typename T>
		inline bool castCmp(const LuaValue& lhs, const LuaValue& rhs)
			requires(std::is_convertible_v<LuaValue, T>)
		{
			// floating point comparisons are safe here
			// because we need strict equality
			// NOLINTNEXTLINE(*float-equal)
			return static_cast<T>(lhs) == static_cast<T>(rhs);
		}
	} // namespace

	// Utility operators
	bool LuaValue::operator==(const LuaValue& other) const
	{
		if (type != other.type) { return false; }

		// Here type == other.type
		switch (type)
		{
			case Type::NUMBER:	  return castCmp<number_t>(*this, other);

			case Type::BOOL:	  return castCmp<bool>(*this, other);

			case Type::NIL:		  return true; /* nil = single value */
			case Type::STRING:	  return castCmp<std::string>(*this, other);

			case Type::LUSERDATA:
			case Type::USERDATA:
			case Type::CDATA:	  return castCmp<ptr_t>(*this, other);

			default:			  return false;
		}
	}

	auto LuaValue::toTable() const -> table_t
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(type == Type::TABLE)

		return std::get<table_t>(value);
	}

	// Utility functions
	std::string LuaValue::toString(const LuaValue& value)
	{
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
				return std::format("cdata<? ? ?>: {:#x}", static_cast<uintptr_t>(value));
			}
			case Type::LUSERDATA:
			case Type::USERDATA:
			{
				return std::format("userdata: {:#x}", static_cast<uintptr_t>(value));
			}
			case Type::STRING:
			{
				return static_cast<std::string>(value);
			}
			case Type::TABLE:
			{
				// Recursively prints the table
				std::string output;
				output += "{ ";

				bool first = true;
				for (const auto& [tbl_key, tbl_val] : value.toTable())
				{
					if (!first) { output += ", "; }
					first = false;

					output += std::format(
						"['{}'] = '{}'",
						LuaValue::toString(tbl_key),
						LuaValue::toString(tbl_val)
					);
				}
				output += "}";

				return output;
			}
			default:
			{
				throw ENGINE_EXCEPTION(std::format(
					"Cannot convert type '{}' to string!",
					static_cast<int>(value.type)
				));
			}
		}
	}

	LuaValue LuaValue::getTableValue(const LuaValue& key) const
	{
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(type == Type::TABLE)
		ENGINE_EXCEPTION_ON_ASSERT(
			key.type != Type::NIL,
			"PIL 2.5 - Tables states that 'nil' is not a valid key!"
		)

		const auto& as_table = std::get<table_t>(value);
		// O(N) comparisons worst case
		for (const auto& [k, v] : as_table)
		{
			if (key == k) { return v; }
		}

		// Return nil
		return {nullptr};
	}
} // namespace Engine::Scripting
