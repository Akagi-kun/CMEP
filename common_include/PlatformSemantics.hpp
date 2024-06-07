#pragma once

#if defined(_MSC_VER)
    // MSVC
    //  for msvc-like semantics use function-only exports
    #define SEMANTICS_MSVC

    #define CMEP_EXPORT_CLASS

    #if !defined(CMEP_ABI_IMPORT)
        #define CMEP_EXPORT __declspec(dllexport)
    #else
        #define CMEP_EXPORT __declspec(dllimport)
    #endif

#elif defined(__GNUC__)
    // GCC
    //  for unix-like semantics we want the class whole to be external  
    #define SEMANTICS_UNIXLIKE

    #if !defined(CMEP_ABI_IMPORT)
        #define CMEP_EXPORT

        #define CMEP_EXPORT_CLASS __attribute__((visibility("default")))

    #else
        #define CMEP_EXPORT

        #define CMEP_EXPORT_CLASS __attribute__((visibility("default")))

    #endif

#else
    // Unknown
    // do nothing and hope for the best?
    #define SEMANTICS_UNKNOWN
    #define CMEP_EXPORT
    #define CMEP_EXPORT_CLASS
    #pragma warning Unknown dynamic link import/export semantics.

#endif
