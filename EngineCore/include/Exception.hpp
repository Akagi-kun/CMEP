#pragma once

#include "PlatformSemantics.hpp"

#include <exception>
#include <source_location>
#include <string>

namespace Engine
{
	class CMEP_EXPORT_CLASS Exception : public std::exception
	{
	public:
		CMEP_EXPORT Exception(
			const std::string&	 with_message,
			std::source_location location = std::source_location::current()
		)
			: message(generateWhat(with_message, location))
		{}

		[[nodiscard]] CMEP_EXPORT const char* what() const noexcept override
		{
			return message.c_str();
		}

	private:
		std::string message;

		static CMEP_EXPORT std::string generateWhat(
			const std::string&	 with_message,
			std::source_location location
		);
	};

	CMEP_EXPORT std::string unrollExceptions(const std::exception& caught_exception);
} // namespace Engine

// NOLINTBEGIN(*unused-macros)
//
// Use as `throw ENGINE_EXCEPTION(message)`
#define ENGINE_EXCEPTION(message) ::Engine::Exception(message)
//
// Runtime assert, use in places where a regular <cassert> assert() may be necessary
// i.e. when true_expr depends on external inputs that are not a part of the engine itself
#define ENGINE_EXCEPTION_ON_ASSERT(true_expr, message)                                   \
	if ((true_expr) == false)                                                            \
	{                                                                                    \
		throw ENGINE_EXCEPTION(                                                          \
			std::string("Failed assertion '" #true_expr "' ") + (message)                \
		);                                                                               \
	}
#define ENGINE_EXCEPTION_ON_ASSERT_NOMSG(true_expr)                                      \
	ENGINE_EXCEPTION_ON_ASSERT((true_expr), "")
//
// NOLINTEND(*unused-macros)
