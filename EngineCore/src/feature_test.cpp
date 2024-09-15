#include <version>

// Checks for std::format support
#if defined(__cpp_lib_format)
//
// Handle the special case of AppleClang not defining __cpp_lib_format when supporting the
// feature this uses the __has_include extension of clang
#elif defined(__llvm__) && __has_include(<format>)
#	pragma message(                                                                                                      \
		"-- __cpp_lib_format is not defined but compiler is __llvm__ and __has_include(<format>), treating as compatible" \
	)
//
// Immediately error out in all other cases
#else
#	error "__cpp_lib_format is not supported!"
#endif
