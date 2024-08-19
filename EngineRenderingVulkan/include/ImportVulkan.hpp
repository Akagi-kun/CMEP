#pragma once

#pragma warning(push, 2)

// Include vulkan
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp" // IWYU pragma: export

// Include GLFW
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h" // IWYU pragma: export

// Include VMA
#include "vk_mem_alloc.h" // IWYU pragma: export

#pragma warning(pop)
