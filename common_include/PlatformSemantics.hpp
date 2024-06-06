#pragma once

#if defined(_MSC_VER)
    //  MSVC
    #define SEMANTICS_MSVC
    #if !defined(CMEP_ABI_IMPORT)
        #define CMEP_EXPORT __declspec(dllexport)
    #else
        #define CMEP_EXPORT __declspec(dllimport)
    #endif
    // Unused
    #define CMEP_EXPORT_CLASS
#elif defined(__GNUC__)
    //  GCC
    #define SEMANTICS_UNIXLIKE
    #if !defined(CMEP_ABI_IMPORT)
        #define CMEP_EXPORT __attribute__((visibility("default")))
        #define CMEP_EXPORT_CLASS __attribute__((visibility("default")))
    #else
        // TODO: fix gcc abi
        #define CMEP_EXPORT __attribute__((visibility("default")))
        #define CMEP_EXPORT_CLASS __attribute__((visibility("default")))
    #endif
#else
    //  do nothing and hope for the best?
    #define CMEP_EXPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif
