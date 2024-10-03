#pragma once
// IWYU pragma: private; include Base.hpp

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

		static CMEP_EXPORT std::string
		generateWhat(const std::string& with_message, std::source_location location);
	};

	[[nodiscard]] CMEP_EXPORT std::string
							  unrollExceptions(const std::exception& caught_exception);
} // namespace Engine

// NOLINTBEGIN(*unused-macros)
/**
 * @brief Use as @code throw ENGINE_EXCEPTION(message) @endcode
 */
#define ENGINE_EXCEPTION(message) ::Engine::Exception(message)

/**
 * @brief Runtime assert, use when true_expr depends on external inputs that are not part of the engine itself
 */
#define ENGINE_EXCEPTION_ON_ASSERT(true_expr, message)                                   \
	if (static_cast<bool>(true_expr) == false)                                           \
	{                                                                                    \
		throw ENGINE_EXCEPTION(                                                          \
			std::string("Failed assertion '" #true_expr "' ") + (message)                \
		);                                                                               \
	}
#define ENGINE_EXCEPTION_ON_ASSERT_NOMSG(true_expr)                                      \
	ENGINE_EXCEPTION_ON_ASSERT((true_expr), "")
// NOLINTEND(*unused-macros)
