/**
 * @file
 * @brief Defines macros describing the compiler semantics
 */
#pragma once

/**
 * @def CMEP_EXPORT
 * Mark function as exported across the shared lib boundary.
 * If used on a member function, also use @ref CMEP_EXPORT_CLASS to export the class
 */
/**
 * @def CMEP_EXPORT_CLASS
 * Mark class as exported across the shared lib boundary.
 * @ref CMEP_EXPORT should be used on member functions that should also be exported.
 */

#if defined(_MSC_VER)
// Windows
//  for msvc-like semantics use function-only exports
#	define SEMANTICS_MSVC

#	define CMEP_EXPORT_CLASS
#	if !defined(CMEP_ABI_IMPORT)
#		define CMEP_EXPORT __declspec(dllexport)
#	else
#		define CMEP_EXPORT __declspec(dllimport)
#	endif

#elif defined(__GNUC__)
// Unix-like
//  for unix-like semantics we want the class whole to be external
#	define SEMANTICS_UNIXLIKE

#	define CMEP_EXPORT
#	if !defined(CMEP_ABI_IMPORT)
#		define CMEP_EXPORT_CLASS __attribute__((visibility("default")))
#	else
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

// Used for #if checks
#ifndef NDEBUG
#	define SEMANTICS_IS_DEBUG 1
#else
#	define SEMANTICS_IS_DEBUG 0
#endif
