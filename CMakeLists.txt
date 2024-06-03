cmake_minimum_required(VERSION 3.5)
project(CMEP)

enable_testing()

set(builddir ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message("-- Build directory is: " ${builddir})
set(EXECUTABLE_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${builddir} CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${builddir} CACHE PATH "Build directory" FORCE)

add_subdirectory(external/glfw)

include_directories(external/glm)

message("-- Build type is: "${CMAKE_BUILD_TYPE})

if(MSVC)
    add_compile_options(/std:c++17 /MP /wd4996 /wd4251)

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("-- Building Debug")
		add_compile_definitions("DEBUG=1")
		add_compile_options(/Od /DEBUG:FULL)
	else()
		message("-- Building Release")
		add_compile_options(/O1)
	endif()
else()
	add_compile_options(-std=c++17)
	add_link_options(-lpthread)
	if(NOT APPLE)
		add_compile_options(--coverage)
		add_link_options(--coverage -lgcov)
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("-- Building Debug")
		add_compile_definitions("DEBUG=1")
		add_compile_options(-g3)
	else()
		message("-- Building Release")
		add_compile_options()
	endif()
endif()

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)

add_subdirectory(rungame)
add_dependencies(rungame glfw)

if(NOT MSVC)
	if(NOT APPLE)
		message("-- Will build lua for Linux")
		add_custom_target(lua_build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/build_lua.sh linux)
	else()
		message("-- Will build lua for MacOS")
		add_custom_target(lua_build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/build_lua.sh macosx)
	endif()
else()
	message("-- Will copy lua from prebuilt (Windows)")
	add_custom_target(lua_build xcopy /s /e /i /y /f "external\\lua-prebuilt\\lua54.dll" "build\\lua54.dll" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
add_dependencies(EngineCore lua_build)
add_dependencies(rungame lua_build)

set(TESTS_TO_RUN
	vulkan_init_cleanup
	vulkan_draw_frame
	vulkan_buffer_create_cleanup

	logging_init_release
	logging_log
	logging_simplelog
	logging_map_thread_simplelog

	engine
	)

add_subdirectory(runtest)
foreach(test_to_run ${TESTS_TO_RUN})
	add_test(NAME runtest_${test_to_run} COMMAND runtest ${test_to_run} WORKING_DIRECTORY .\\build\\)
endforeach()

add_subdirectory(examples)

#if(NOT APPLE)
#	file(COPY external/lua-prebuilt/liblua54.so DESTINATION build)
#else()
#	file(COPY external/lua-5.4.6/src/lua54.so DESTINATION build/liblua54.so)
#endif()