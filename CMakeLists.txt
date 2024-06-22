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

# Global compile options
if(MSVC)
	add_compile_options(/std:c++17 /MP /EHs)
	add_link_options(/DEBUG /INCREMENTAL:NO)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:/Od>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:/O2>)

else()
	add_compile_options(-std=c++17)
	add_link_options(-lpthread -lstdc++)

	# Debug only
	add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)
	add_compile_options($<$<CONFIG:Debug>:-g3>)

	# Release only
	add_compile_options($<$<CONFIG:Release>:-O2>)

endif()

add_subdirectory(EngineCore)
add_dependencies(EngineCore glfw)

add_subdirectory(rungame)
add_dependencies(rungame glfw)

add_subdirectory(external)

add_dependencies(EngineCore lua_build)

add_dependencies(rungame EngineCore)

#set(TESTS_TO_RUN
#	vulkan_init_cleanup
#	vulkan_draw_frame
#	vulkan_buffer_create_cleanup
#
#	logging_init_release
#	logging_log
#	logging_simplelog
#	logging_map_thread_simplelog
#
#	engine
#	)

#add_subdirectory(runtest)
#foreach(test_to_run ${TESTS_TO_RUN})
#	add_test(NAME runtest_${test_to_run} COMMAND runtest ${test_to_run} WORKING_DIRECTORY .\\build\\)
#endforeach()

add_subdirectory(examples)
