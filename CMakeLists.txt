cmake_minimum_required(VERSION 3.12)
project(CMEP LANGUAGES CXX)

#enable_testing()

find_package(Vulkan REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VAR_BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
message("-- Build directory is: " ${VAR_BUILD_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY})

if(MSVC)
# TODO: Remove /wd4996 after deprecation fixes
	set(ENGINE_COMPILE_OPTIONS /W3 /WX /wd4996 /wd4068)
else()
	set(ENGINE_COMPILE_OPTIONS -Wall -Wpedantic -Wno-unknown-pragmas) # -Wextra -Werror

	if(APPLE)
		list(APPEND ENGINE_COMPILE_OPTIONS -Wno-nullability -Wno-nullability-completeness -Wno-nullability-extension)
	endif()
endif()

# Global compile options
if(MSVC)
	add_compile_options(/MP /EHs /Zi)
	add_link_options(/DEBUG:FULL)

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

add_subdirectory(docs)

set(CMAKE_CONFIGURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(common_include/cmake_cfg.hpp.in common_include/cmake_cfg.hpp)

add_subdirectory(external)

add_subdirectory(EngineBase)

add_subdirectory(EngineLogging)
add_subdirectory(EngineRenderingVulkan)

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)
add_dependencies(EngineCore lua_build)

add_subdirectory(rungame)
add_dependencies(rungame glfw)
add_dependencies(rungame EngineCore)

add_subdirectory(examples)
