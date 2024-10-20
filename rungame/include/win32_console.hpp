/**
 * @file
 * @brief Exposes Win32-specific console-related functions
 */
#pragma once

#include "PlatformSemantics.hpp"

// Only ever expose this file's contents if on Windows
#if defined(SEMANTICS_MSVC)
// Include minimum Win32
#	define WIN32_LEAN_AND_MEAN
// Windows.h defines min() and max() macros, which break std::min and std::max
#	define NOMINMAX
#	include <Windows.h>

namespace
{
	/**
	 * @brief Initializes Win32 console to work with colors
	 */
	inline void initConsoleWin32()
	{
		HANDLE my_console = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD  dw_mode	  = 0;
		GetConsoleMode(my_console, &dw_mode);
		dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(my_console, dw_mode);
	}
} // namespace
#endif
