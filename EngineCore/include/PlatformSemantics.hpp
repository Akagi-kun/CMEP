#pragma once

#if defined(_MSC_VER)
    //  MSVC
    #define CMEP_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
    //  GCC
    #define CMEP_EXPORT __attribute__((visibility("default")))
#else
    //  do nothing and hope for the best?
    #define CMEP_EXPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif
