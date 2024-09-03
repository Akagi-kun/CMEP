#include "ConsoleColors.hpp"

namespace Logging::Console
{
	const char* WHITE_FG  = "\033[0;97m";			 // info
	const char* GREEN_FG  = "\033[0;92m";			 // success
	const char* YELLOW_FG = "\033[0;93m";			 // warning
	const char* RED_FG	  = "\033[38;2;255;10;10m";	 // error
	const char* BLUE_FG	  = "\033[38;2;66;205;255m"; // exception
	const char* GRAY_FG	  = "\033[0;90m";			 // debug
	const char* RESET_FG  = "\033[0m";				 // reset
} // namespace Logging::Console
