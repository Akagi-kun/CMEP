#pragma once

#include <exception>
#include <string>

namespace Engine
{
	class Exception : public std::exception
	{
	public:
		Exception(const char* with_file, uint_least32_t with_line, const std::string& with_message)
			: message(
				  std::string(with_file) + " at line " + std::to_string(with_line) + ":\n\t\t" +
				  with_message
			  )
		{
		}

		[[nodiscard]] const char* what() const noexcept override
		{
			return message.c_str();
		}

	private:
		std::string message;
	};
} // namespace Engine

// NOLINTNEXTLINE(*unused-macros)
#define ENGINE_EXCEPTION(message) ::Engine::Exception(__FILE__, __LINE__, message)
