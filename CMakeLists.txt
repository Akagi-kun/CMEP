cmake_minimum_required(VERSION 3.0)
project(CMEP)

set(builddir ${CMAKE_CURRENT_SOURCE_DIR}/build/)
set(EXECUTABLE_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${builddir} CACHE PATH "Build directory" FORCE)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build glfw shared libs")
add_subdirectory(external/glfw)
add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)

add_subdirectory(rungame)

add_subdirectory(examples)
