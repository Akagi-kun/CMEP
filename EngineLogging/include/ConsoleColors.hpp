#pragma once

#include <string_view>

namespace Logging::Console
{
	constexpr std::string_view WHITE_FG	 = "\033[0;97m";			// info
	constexpr std::string_view GREEN_FG	 = "\033[0;92m";			// success
	constexpr std::string_view YELLOW_FG = "\033[0;93m";			// warning
	constexpr std::string_view RED_FG	 = "\033[38;2;255;10;10m";	// error (rgb)
	constexpr std::string_view BLUE_FG	 = "\033[38;2;66;205;255m"; // exception (rgb)
	constexpr std::string_view GRAY_FG	 = "\033[0;90m";			// debug
	constexpr std::string_view RESET_FG	 = "\033[0m";				// reset
} // namespace Logging::Console
