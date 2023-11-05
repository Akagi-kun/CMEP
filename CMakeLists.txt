cmake_minimum_required(VERSION 3.5)
project(CMEP)

if(MSVC)
    add_compile_options(/std:c++17 /wd4996 /wd4251)
else()
	add_compile_definitions("DEBUG=$<CONFIG:Debug>")
	add_compile_options(-std=c++17 -lpthread)
endif()

set(builddir ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(${builddir})
set(EXECUTABLE_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${builddir} CACHE PATH "Build directory" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)

add_subdirectory(rungame)
add_dependencies(rungame glfw)

enable_testing()
add_subdirectory(runtest)
add_test(NAME vulkan_test COMMAND test_vulkan)
#add_dependencies(runtest glfw)

add_subdirectory(examples)

file(COPY external/lua/lua54.dll DESTINATION build)
file(COPY external/lua/liblua54.so DESTINATION build)