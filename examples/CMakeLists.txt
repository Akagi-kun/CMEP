message(STATUS "[EXAMPLES] Source dir is: " ${CMAKE_SOURCE_DIR})

file(STRINGS 
	 ${CMAKE_SOURCE_DIR}/shader_library/files.txt
	 VAR_AVAILABLE_SHADERS
	 )

function(add_example_directory VAR_DIRECTORY)
	set(VAR_GAME_DIRECTORY ${VAR_BUILD_DIRECTORY}/game)

	# Copy example
	add_custom_target(${VAR_DIRECTORY}-copy-folder)
	add_custom_command(TARGET ${VAR_DIRECTORY}-copy-folder PRE_BUILD
	                    COMMAND ${CMAKE_COMMAND} -E
	                    copy_directory_if_different examples/${VAR_DIRECTORY} ${VAR_GAME_DIRECTORY}
	                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	                    )

	# Copy in all shaders available in the shader_library directory
	add_custom_target(${VAR_DIRECTORY}-copy-shaders)
	foreach(file ${VAR_AVAILABLE_SHADERS})
		# Fix: POST_BUILD 
		add_custom_command(TARGET ${VAR_DIRECTORY}-copy-shaders POST_BUILD
						   COMMAND ${CMAKE_COMMAND} -E
						   copy ${file} ${VAR_GAME_DIRECTORY}/shaders
						   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shader_library
						   )
	endforeach()

	add_dependencies(${VAR_DIRECTORY}-copy-shaders ${VAR_DIRECTORY}-copy-folder)

	add_custom_target(${VAR_DIRECTORY})
	add_dependencies(${VAR_DIRECTORY} ${VAR_DIRECTORY}-copy-shaders)
endfunction()

add_example_directory(floppybirb)
add_example_directory(voxelgame)
