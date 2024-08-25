// Provides a source file for once-per-project instantiation of libraries

// Provide storage for the dynamic dispatcher
#include "vulkan/vulkan.hpp"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

// Uncomment this to log all allocations to stdout
/*
#define VMA_DEBUG_LOG_FORMAT(format, ...)                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		printf((format), __VA_ARGS__);                                                                                 \
		printf("\n");                                                                                                  \
	} while (false)
*/
// Include VMA implementation
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
