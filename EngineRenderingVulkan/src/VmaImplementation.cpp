// Uncomment this to log all allocations to stdout
/*
#define VMA_DEBUG_LOG_FORMAT(format, ...)                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		printf((format), __VA_ARGS__);                                                                                 \
		printf("\n");                                                                                                  \
	} while (false)
*/
//

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
