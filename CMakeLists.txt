cmake_minimum_required(VERSION 3.12)
project(CMEP LANGUAGES CXX)

# Include the Vulkan SDK
find_package(Vulkan REQUIRED COMPONENTS)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Setup build directory to stably point to ./build/
set(VAR_BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
message(STATUS "Build directory is: " ${VAR_BUILD_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY})

# Check Inter-Procedural Optimization support
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_support_result OUTPUT ipo_support_error)

if(NOT ipo_support_result)
	message(WARNING "LTO unsupported, disabled; Reason: ${ipo_support_error}")
endif()

# Generator expression that evaluates to 1 if LTO is supported and expected to be used
set(ENGINE_DO_LTO $<BOOL:$<AND:$<BOOL:${ipo_support_result}>,$<CONFIG:Release>>>)

# EngineCore compile options
if(MSVC)
	# TODO: Remove /wd4996 after deprecation fixes
	set(ENGINE_COMPILE_OPTIONS /W3 /WX /wd4068)
else()
	set(ENGINE_COMPILE_OPTIONS -Wall -Wpedantic -Wno-unknown-pragmas) # -Wextra -Werror

	# We don't use nullability specifiers, which are enabled by default on AppleClang, disable them
	if(APPLE)
		list(APPEND ENGINE_COMPILE_OPTIONS -Wno-nullability -Wno-nullability-completeness -Wno-nullability-extension)
	endif()
endif()

# Global compile options
if(MSVC)
	add_compile_options(/MP /EHs /Zi)
	# Include full PDB
	add_link_options(/DEBUG:FULL)
	# Add /LTCG if using LTO on a Release build
	add_link_options($<${ENGINE_DO_LTO}:/LTCG>)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:/Od>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:/O2>)

else()
	add_compile_options(-g)
	add_link_options(-lpthread -lstdc++)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:-g3>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:-O2>)

endif()

# Docs
add_subdirectory(docs)

# Build config header
set(CMAKE_CONFIGURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(common_include/cmake_cfg.hpp.in common_include/cmake_cfg.hpp)

# External dependencies
add_subdirectory(external)

# EngineCore components
add_subdirectory(EngineBase)

add_subdirectory(EngineLogging)
add_subdirectory(EngineRenderingVulkan)

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)
add_dependencies(EngineCore lua_build)

add_subdirectory(rungame)
add_dependencies(rungame glfw)
add_dependencies(rungame EngineCore)

# Use LTO only in Release build
if(ipo_support_result)
	message(STATUS "LTO enabled for Release builds")
	set_target_properties(EngineBase EngineLogging EngineRenderingVulkan EngineCore
						  PROPERTIES
						  INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
						  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# Examples
add_subdirectory(examples)
