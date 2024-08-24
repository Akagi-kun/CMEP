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
#define MACRO_STR(macro)  MACRO_QUOTE(macro)

// This absolute abomination of macros defines BUILDINFO_COMPILED_BY
// and SEMANTICS_COMPILER_* which can be used instead of
// common_include/PlatformSemantics.hpp SEMANTICS_*
// to get more precise semantic checks (not extra reliable though)
//
// MSVC compiler
#if defined(_MSC_VER)
#	pragma clang diagnostic ignored "-W#pragma-messages"
#	pragma message("Compiler MSVC detected")
#	define SEMANTICS_COMPILER_MSVC
#	define BUILDINFO_COMPILED_BY "MSVC " MACRO_STR(_MSC_FULL_VER) "." MACRO_STR(_MSC_BUILD)
// Any GNU-like compiler
#elif defined(__GNUC__)
#	define SEMANTICS_COMPILER_ANY_GNU
//	Compilers that use LLVM as backend
#	if defined(__llvm__)
#		pragma message("Compiler LLVM detected")
#		define SEMANTICS_COMPILER_ANY_LLVM
//		Clang with LLVM backend
#		if defined(__clang__)
#			pragma message("Compiler LLVM-clang detected")
#			define SEMANTICS_COMPILER_LLVM_CLANG
#			define BUILDINFO_COMPILED_BY                                                                              \
				"LLVM-clang " MACRO_STR(__clang_major__) "." MACRO_STR(__clang_minor__                                 \
				) "." MACRO_STR(__clang_patchlevel__);
//		An GNU-like compiler with LLVM that is not clang is likely to be LLVM GCC
#		else
#			pragma message("Compiler LLVM-GCC detected")
#			define SEMANTICS_COMPILER_LLVM_GCC
#			define BUILDINFO_COMPILED_BY                                                                              \
				"LLVM-GCC " MACRO_STR(__GNUC__) "." MACRO_STR(__GNUC_MINOR__) "." MACRO_STR(__GNUC_PATCHLEVEL__)
#		endif
//	An GNU-like compiler that is not using LLVM is likely to be pure GCC
#	else
#		pragma message("Compiler GCC detected")
#		define SEMANTICS_COMPILER_GCC
#		define BUILDINFO_COMPILED_BY                                                                                  \
			"GCC " MACRO_STR(__GNUC__) "." MACRO_STR(__GNUC_MINOR__) "." MACRO_STR(__GNUC_PATCHLEVEL__);
#	endif
// Any other compiler will simply be Unknown
#else
#	pragma warning "Compiler could not be identified"
#	define SEMANTICS_COMPILER_UNKNOWN
#	define BUILDINFO_COMPILED_BY "Unknown"
#endif

namespace Engine
{
	// Build string
	const char* const buildinfo_build = "CMEP EngineCore " __TIME__ " " __DATE__ " build, configured " BUILDCONFIG
										" " MACRO_STR(IS_DEBUG);

	// Name and version of compiler
	const char* const buildinfo_compiledby = BUILDINFO_COMPILED_BY;
} // namespace Engine
