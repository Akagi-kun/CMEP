#pragma once

//
// This file generates compile-time strings
// detailing the build configuration
//

#ifdef DEBUG
#	define IS_DEBUG 1
#else
#	define IS_DEBUG 0
#endif

#if IS_DEBUG == 1
#	define BUILDCONFIG "DEBUG"
#else
#	define BUILDCONFIG "RELEASE"
#endif

#define MACRO_QUOTE(name) #name
#define MACRO_STR(macro) MACRO_QUOTE(macro)

namespace Engine
{
	// Build string
	const char* const buildinfo_build = "CMEP EngineCore " __TIME__ " " __DATE__ " build, configured " BUILDCONFIG
										" " MACRO_STR(IS_DEBUG);

	// Name and version of compiler
	extern const char* const buildinfo_compiledby;

#if defined(_MSC_VER)
#	pragma clang diagnostic ignored "-W#pragma-messages"
#	pragma message("Compiler MSVC detected")
	const char* const buildinfo_compiledby = "MSVC " MACRO_STR(_MSC_FULL_VER) "." MACRO_STR(_MSC_BUILD);
#elif defined(__GNUC__)
#	pragma message("Compiler GNU-like detected")
#	if defined(__llvm__)
#		pragma message("Compiler LLVM detected")
#		if defined(__clang__)
#			pragma message("Compiler LLVM-clang detected")
	const char* const buildinfo_compiledby = "LLVM-clang " MACRO_STR(__clang_major__) "." MACRO_STR(__clang_minor__
	) "." MACRO_STR(__clang_patchlevel__);
#		else
#			pragma message("Compiler LLVM-gcc detected")
	const char* const buildinfo_compiledby = "LLVM-GCC " MACRO_STR(__GNUC__) "." MACRO_STR(__GNUC_MINOR__
	) "." MACRO_STR(__GNUC_PATCHLEVEL__);
#		endif
#	else
#		pragma message("Compiler gcc detected")
	const char* const buildinfo_compiledby = "GCC " MACRO_STR(__GNUC__) "." MACRO_STR(__GNUC_MINOR__
	) "." MACRO_STR(__GNUC_PATCHLEVEL__);
#	endif
#else
#	pragma warning "Compiler could not be identified"
	const char* const buildinfo_compiledby = "Nil";
#endif
} // namespace Engine
