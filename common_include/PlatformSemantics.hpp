#pragma once

#include <version>

// Checks for std::format support
#if defined(__cpp_lib_format)
//
// Handle the special case of AppleClang not defining __cpp_lib_format when supporting the feature
// this uses the __has_include extension of clang
#elif defined(__llvm__) && __has_include(<format>)
#	pragma message                                                                                \
		"__cpp_lib_format is not defined but compiler is __llvm__ and __has_include(<format>), treating as compatible"
//
// Immediately error out in all other cases
#else
#	error "__cpp_lib_format is not supported!"
#endif

#if defined(_MSC_VER)
// MSVC
//  for msvc-like semantics use function-only exports
#	define SEMANTICS_MSVC

#	if !defined(CMEP_ABI_IMPORT)
#		define CMEP_EXPORT __declspec(dllexport)
#		define CMEP_EXPORT_CLASS
#	else
#		define CMEP_EXPORT __declspec(dllimport)
#		define CMEP_EXPORT_CLASS
#	endif

#elif defined(__GNUC__)
// GCC
//  for unix-like semantics we want the class whole to be external
#	define SEMANTICS_UNIXLIKE

#	if !defined(CMEP_ABI_IMPORT)
#		define CMEP_EXPORT
#		define CMEP_EXPORT_CLASS __attribute__((visibility("default")))

#	else
#		define CMEP_EXPORT
#		define CMEP_EXPORT_CLASS __attribute__((visibility("default")))
#	endif

#else
// Unknown
// do nothing and hope for the best?
#	define SEMANTICS_UNKNOWN
#	define CMEP_EXPORT
#	define CMEP_EXPORT_CLASS
#	pragma warning Unknown dynamic link import / export semantics.

#endif
