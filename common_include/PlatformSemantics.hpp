#pragma once

#if defined(_MSC_VER)
    //  MSVC
    #if !defined(CMEP_ABI_IMPORT)
        #define CMEP_EXPORT __declspec(dllexport)
    #else
        #define CMEP_EXPORT __declspec(dllimport)
    #endif
#elif defined(__GNUC__)
    //  GCC
    #if !defined(CMEP_ABI_IMPORT)
        #define CMEP_EXPORT __attribute__((visibility("default")))
    #else
        // TODO: fix gcc abi
        #define CMEP_EXPORT __attribute__((visibility("default")))
    #endif
#else
    //  do nothing and hope for the best?
    #define CMEP_EXPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif
