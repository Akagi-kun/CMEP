#pragma once

#include "PlatformSemantics.hpp"
#include "cmake_cfg.hpp"

#include <exception>
#include <filesystem>
#include <string>

namespace Engine
{
	CMEP_EXPORT_CLASS class Exception : public std::exception
	{
	public:
		CMEP_EXPORT Exception(
			const std::filesystem::path& with_file,
			uint_least32_t				 with_line,
			const std::string&			 with_message
		)
			: message(std::format(
				  "{}@{}: {}",
				  // Make relative to source directory, may not be foolproof!
				  with_file.lexically_relative(CMAKE_CONFIGURE_SOURCE_DIR).string(),
				  std::to_string(with_line),
				  with_message
			  ))
		{
		}

		[[nodiscard]] CMEP_EXPORT const char* what() const noexcept override
		{
			return message.c_str();
		}

	private:
		std::string message;
	};

	CMEP_EXPORT std::string UnrollExceptions(const std::exception& caught_exception);
} // namespace Engine

// NOLINTBEGIN(*unused-macros)
#define ENGINE_EXCEPTION(message) ::Engine::Exception(__FILE__, __LINE__, message)
#define ENGINE_EXCEPTION_ON_ASSERT(true_expr, message)                                             \
	if ((true_expr) == false)                                                                      \
	{                                                                                              \
		throw ENGINE_EXCEPTION(std::string("Failed assertion '" #true_expr "' ") + (message));     \
	}
#define ENGINE_EXCEPTION_ON_ASSERT_NOMSG(true_expr) ENGINE_EXCEPTION_ON_ASSERT((true_expr), "")
// NOLINTEND(*unused-macros)
