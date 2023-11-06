cmake_minimum_required(VERSION 3.5)
project(CMEP)

enable_testing()

set(builddir ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(${builddir})
set(EXECUTABLE_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${builddir} CACHE PATH "Build directory" FORCE)

add_subdirectory(external/glfw)

if(MSVC)
    add_compile_options(/std:c++17 /wd4996 /wd4251)
else()
	add_compile_definitions("DEBUG=$<CONFIG:Debug>")
	add_compile_options(-std=c++17 --coverage)
	add_link_options(-lpthread --coverage)
	if(NOT APPLE)
		add_link_options(-lgcov)
	endif()
endif()

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)

add_subdirectory(rungame)
add_dependencies(rungame glfw)

set(TESTS_TO_RUN
	vulkan_init_cleanup
	vulkan_draw_frame
	vulkan_buffer_create_cleanup
	)

add_subdirectory(runtest)
foreach(test_to_run ${TESTS_TO_RUN})
	add_test(NAME runtest_${test_to_run} COMMAND runtest ${test_to_run} WORKING_DIRECTORY build/)
endforeach()

add_subdirectory(examples)

file(COPY external/lua/lua54.dll DESTINATION build)
file(COPY external/lua/liblua54.so DESTINATION build)
