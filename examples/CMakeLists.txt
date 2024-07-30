cmake_minimum_required(VERSION 3.10)
project(examples)

message("-- [EXAMPLES] Source directory is: " ${CMAKE_SOURCE_DIR})

function(add_example_directory VAR_DIRECTORY)
	if(MSVC)
	    add_custom_target(${VAR_DIRECTORY}-compile-shaders compile.bat WORKING_DIRECTORY ${VAR_DIRECTORY}/shaders/vulkan)
	else()
	    add_custom_target(${VAR_DIRECTORY}-compile-shaders ./compile.sh WORKING_DIRECTORY ${VAR_DIRECTORY}/shaders/vulkan)
	endif()

	add_custom_target(${VAR_DIRECTORY}-perform-copy)
	add_custom_command(TARGET ${VAR_DIRECTORY}-perform-copy
	                    COMMAND ${CMAKE_COMMAND} -E
	                    copy_directory_if_different examples/${VAR_DIRECTORY} ${VAR_BUILD_DIRECTORY}/game
	                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	                    )
	
	add_custom_command(TARGET ${VAR_DIRECTORY}-perform-copy
	                   POST_BUILD
	                   COMMAND ${CMAKE_COMMAND} -E
	                   rm
	                   shaders/vulkan/compile.bat
	                   shaders/vulkan/cleanup.bat
	                   shaders/vulkan/compile.sh
	                   shaders/vulkan/cleanup.sh
	                   WORKING_DIRECTORY ${VAR_BUILD_DIRECTORY}/game
	                   )

	add_custom_target(${VAR_DIRECTORY})
	add_dependencies(${VAR_DIRECTORY} ${VAR_DIRECTORY}-compile-shaders ${VAR_DIRECTORY}-perform-copy)
endfunction()

add_example_directory(floppybirb)
add_example_directory(testgame)
