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

# Global compile options
if(MSVC)
	add_compile_options(/std:c++17 /MP /EHs /Zi)
	add_link_options(/DEBUG:FULL /INCREMENTAL:NO /OPT:REF)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:/Od>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:/O2>)

else()
	add_compile_options(-std=c++17 -g)
	add_link_options(-lpthread -lstdc++)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:-g3>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:-O2>)

endif()

add_subdirectory(external)

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)
add_dependencies(EngineCore lua_build)

add_subdirectory(rungame)
add_dependencies(rungame glfw)
add_dependencies(rungame EngineCore)

add_subdirectory(examples)
