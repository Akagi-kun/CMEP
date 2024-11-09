#pragma once

#include <string_view>

namespace Logging::Console
{
	using color_t = std::string_view;

	constexpr color_t WHITE_FG	= "\033[0;97m";			   // info
	constexpr color_t GREEN_FG	= "\033[0;92m";			   // success
	constexpr color_t YELLOW_FG = "\033[0;93m";			   // warning
	constexpr color_t RED_FG	= "\033[38;2;255;10;10m";  // error (rgb)
	constexpr color_t BLUE_FG	= "\033[38;2;66;205;255m"; // exception (rgb)
	constexpr color_t GRAY_FG	= "\033[0;90m";			   // debug
	constexpr color_t RESET_FG	= "\033[0m";			   // reset
} // namespace Logging::Console
