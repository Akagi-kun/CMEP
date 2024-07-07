#pragma once

#pragma warning(push, 2)

// Include GLFW with Vulkan
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h" // IWYU pragma: export

// Include VMA
#include "vk_mem_alloc.h"

#pragma warning(pop)
