#pragma once

#include <exception>
#include <string>

namespace Engine
{
	class exception : public std::exception
	{
	public:
		exception(const char* with_file, uint_least32_t with_line, const std::string& with_message)
			: message(std::string(with_file) + " at line " + std::to_string(with_line) + ":\n\t\t" + with_message)
		{
		}

		[[nodiscard]] const char* what() const override
		{
			return message.c_str();
		}

	private:
		std::string message;
	};
} // namespace Engine

// NOLINTNEXTLINE(*unused-macros)
#define ENGINE_EXCEPTION(message) ::Engine::exception(__FILE__, __LINE__, message)
