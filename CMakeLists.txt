cmake_minimum_required(VERSION 3.5)
project(CMEP)

#enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VAR_BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
message("-- Build directory is: " ${VAR_BUILD_DIRECTORY})
set(EXECUTABLE_OUTPUT_PATH ${VAR_BUILD_DIRECTORY} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${VAR_BUILD_DIRECTORY} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VAR_BUILD_DIRECTORY} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VAR_BUILD_DIRECTORY} CACHE PATH "Build directory" FORCE)

if(MSVC)
# TODO: Remove /wd4996 after deprecation fixes
	set(ENGINE_COMPILE_OPTIONS /W3 /WX /wd4996 /wd4068) # /std:c++17
else()
	set(ENGINE_COMPILE_OPTIONS -Wall -Wno-unknown-pragmas -Wno-nullability -Wno-nullability-completeness) # -Wextra -Wpedantic -Werror -std=c++17
endif()

if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		set(ENGINE_VULKAN_LINK_PATH $ENV{VULKAN_SDK}/Lib)
		set(ENGINE_VULKAN_INCLUDE_PATH $ENV{VULKAN_SDK}/Include)
	else()
		set(ENGINE_VULKAN_LINK_PATH $ENV{VULKAN_SDK}/lib)
		set(ENGINE_VULKAN_INCLUDE_PATH $ENV{VULKAN_SDK}/include)
	endif()
else()
	message("Warning! Vulkan SDK environment variable not found!")
endif()

# Global compile options
if(MSVC)
	add_compile_options(/MP /EHs /Zi)
	add_link_options(/DEBUG:FULL /INCREMENTAL:NO /OPT:REF)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:/Od>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:/O1>)

else()
	add_compile_options(-g)
	add_link_options(-lpthread -lstdc++)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:-g3>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:-O1>)

endif()

add_subdirectory(external)

add_subdirectory(EngineLogging)
add_subdirectory(EngineRenderingVulkan)

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)
add_dependencies(EngineCore lua_build)

add_subdirectory(rungame)
add_dependencies(rungame glfw)
add_dependencies(rungame EngineCore)

add_subdirectory(examples)
