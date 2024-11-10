#include "Scripting/LuaValue.hpp"

#include "Scripting/Utility.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Exception.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace Engine::Scripting
{
	namespace
	{
		// Ensure type is Type::TABLE when calling
		// recursive if table contains another table
		LuaValue::value_t luaTableParser(lua_State* state, int stack_index)
		{
			LuaValue::table_t table;
			auto			  abs_index = Utility::relativeToAbsoluteIndex(state, stack_index);

			// Push nil as the first key (iterates whole table)
			lua_pushnil(state);
			while (lua_next(state, abs_index) != 0)
			{
				auto key   = LuaValue(state, -2);
				auto value = LuaValue(state, -1);

				// Keys can be any value except nil per PIL 2.5 - Tables
				EXCEPTION_ASSERT(
					key.type != LuaValue::Type::NIL,
					"PIL 2.5 - Tables states that 'nil' is not a valid key!"
				);

				table.emplace_back(key, value);

				// Pop value, keep key on stack!
				lua_pop(state, 1);
			}

			return table;
		}

		/**
		 * Get the type of a value in the Lua stack
		 *
		 * @param state Lua state
		 * @param stack_index Index in the stack
		 * @return Type of the value
		 */
		LuaValue::Type luaTypeGetter(lua_State* state, int stack_index)
		{
			EXCEPTION_ASSERT(
				!lua_isnone(state, stack_index),
				"Cannot construct LuaValue from LUA_TNONE"
			);

			return static_cast<LuaValue::Type>(lua_type(state, stack_index));
		}

		/**
		 * Get a value from the Lua stack
		 *
		 * @warning This function may fail if @p type is incorrect
		 *
		 * @param state Lua state
		 * @param stack_index Index in the stack
		 * @param type Type of the value
		 * @return The value
		 */
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

				case Type::BOOL:	  return static_cast<bool>(lua_toboolean(state, stack_index));

				default:
				{
					throw ENGINE_EXCEPTION("Reached default in LuaValue ctor!");
				}
			}
		}

		template <typename value_t>
		bool compareCast(const LuaValue& lhs, const LuaValue& rhs)
			requires(std::is_convertible_v<LuaValue, value_t>)
		{
			// floating point comparisons are safe here
			// because we need strict equality
			// NOLINTNEXTLINE(*float-equal)
			return static_cast<value_t>(lhs) == static_cast<value_t>(rhs);
		}
	} // namespace

	LuaValue::LuaValue(lua_State* state, int stack_index)
		: type(luaTypeGetter(state, stack_index)), value(luaValueGetter(state, stack_index, type))
	{}

	void LuaValue::push(lua_State* state) const
	{
		switch (type)
		{
			case Type::STRING:
			{
				lua_pushstring(state, static_cast<string_t>(*this).c_str());
				break;
			}
			case Type::BOOL:
			{
				lua_toboolean(state, static_cast<int>(static_cast<bool>(*this)));
				break;
			}
			case Type::NUMBER:
			{
				lua_pushnumber(state, static_cast<number_t>(*this));
				break;
			}
			case Type::NIL:
			{
				lua_pushnil(state);
				break;
			}
			case Type::LUSERDATA:
			{
				lua_pushlightuserdata(state, static_cast<ptr_t>(*this));
				break;
			}
			default:
			{
				assert(false && "Not implemented!");
			}
		}
	}

	/**
	 * Error message returned when calling a conversion operator or getter function of @ref LuaValue
	 * with @ref LuaValue::type being incompatible with the type the getter expects
	 */
	constexpr std::string_view getter_err = "Called getter with invalid type!";

	// Base getters
	LuaValue::operator string_t() const
	{
		EXCEPTION_ASSERT(type == Type::STRING, getter_err);
		return std::get<string_t>(value);
	}

	LuaValue::operator number_t() const
	{
		EXCEPTION_ASSERT(type == Type::NUMBER, getter_err);
		return std::get<number_t>(value);
	}

	LuaValue::operator ptr_t() const
	{
		EXCEPTION_ASSERT(
			type == Type::USERDATA || type == Type::LUSERDATA || type == Type::CDATA,
			getter_err
		);
		return std::get<ptr_t>(value);
	}

	LuaValue::operator bool() const
	{
		EXCEPTION_ASSERT(type == Type::BOOL, getter_err);
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
			else
			{
				return {};
			}
		};

		return std::visit(supply_data_visitor, value);
	}

	LuaValue::operator uintptr_t() const
	{
		EXCEPTION_ASSERT(
			type == Type::USERDATA || type == Type::LUSERDATA || type == Type::CDATA,
			getter_err
		);
		return reinterpret_cast<uintptr_t>(std::get<void*>(value));
	}

	// Utility operators
	bool LuaValue::operator==(const LuaValue& other) const
	{
		// If types don't match, the values can't either
		if (type != other.type)
		{
			return false;
		}

		// Here type == other.type
		switch (type)
		{
			case Type::NUMBER:	  return compareCast<number_t>(*this, other);

			case Type::BOOL:	  return compareCast<bool>(*this, other);

			case Type::NIL:		  return true; /* nil = single value */
			case Type::STRING:	  return compareCast<std::string>(*this, other);

			case Type::LUSERDATA:
			case Type::USERDATA:
			case Type::CDATA:	  return compareCast<ptr_t>(*this, other);

			// Table does not currently support comparisons
			case Type::TABLE:
			default:			  return false;
		}
	}

	bool LuaValue::operator!=(const LuaValue& other) const
	{
		return !(*this == other);
	}

	auto LuaValue::toTable() const -> table_t
	{
		EXCEPTION_ASSERT(type == Type::TABLE, getter_err);

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
					if (!first)
					{
						output += ", ";
					}
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
				throw ENGINE_EXCEPTION(
					std::format("Cannot convert type '{}' to string!", static_cast<int>(value.type))
				);
			}
		}
	}

	std::optional<LuaValue> LuaValue::getTableValue(const LuaValue& key) const
	{
		assert(type == Type::TABLE && "Tried accessing non-table value as table!");
		EXCEPTION_ASSERT(
			key.type != Type::NIL,
			"'PIL 2.5 - Tables' states that 'nil' is not a valid key!"
		);

		const auto& as_table = std::get<table_t>(value);
		// O(N) comparisons worst case
		for (const auto& [k, v] : as_table)
		{
			if (key == k)
			{
				return v;
			}
		}

		// Return nil
		return {};
	}
} // namespace Engine::Scripting
