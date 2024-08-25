cmake_minimum_required(VERSION 3.10)
project(examples)

message("-- [EXAMPLES] Source dir is: " ${CMAKE_SOURCE_DIR})

file(STRINGS 
	 ${CMAKE_SOURCE_DIR}/shader_library/files.txt
	 VAR_AVAILABLE_SHADERS
	 )

# Set platform dependent compile command
if(MSVC)
	set(VAR_SHADER_COMPILE_COMMAND $ENV{VULKAN_SDK}/Bin/glslc.exe)
else()
	set(VAR_SHADER_COMPILE_COMMAND $ENV{VULKAN_SDK}/bin/glslc)
endif()

# Compile a specific shader
function(compile_shader VAR_DIRECTORY VAR_SHADER)

	# Process path into stem (filename without extension)
	cmake_path(SET PATH_SHADER ${VAR_SHADER})
	cmake_path(GET PATH_SHADER STEM VAR_SHADER_FILENAME)

	message("-- Shader available: " ${VAR_SHADER_FILENAME})

	add_custom_command(TARGET ${VAR_DIRECTORY}-copy-shaders POST_BUILD
					   COMMAND ${VAR_SHADER_COMPILE_COMMAND} ${VAR_SHADER_FILENAME}.glsl -o ${VAR_SHADER_FILENAME}.spv
					   WORKING_DIRECTORY ${VAR_BUILD_DIRECTORY}/game/shaders)
endfunction()

function(add_example_directory VAR_DIRECTORY)
	# Copy example
	add_custom_target(${VAR_DIRECTORY}-copy-folder)
	add_custom_command(TARGET ${VAR_DIRECTORY}-copy-folder PRE_BUILD
	                    COMMAND ${CMAKE_COMMAND} -E
	                    copy_directory_if_different examples/${VAR_DIRECTORY} ${VAR_BUILD_DIRECTORY}/game
	                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	                    )

	# Copy in all shaders available in the shader_library directory
	add_custom_target(${VAR_DIRECTORY}-copy-shaders)
	foreach(file ${VAR_AVAILABLE_SHADERS})
		add_custom_command(TARGET ${VAR_DIRECTORY}-copy-shaders
						   COMMAND ${CMAKE_COMMAND} -E
						   copy ${file} ${VAR_BUILD_DIRECTORY}/game/shaders
						   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shader_library
						   )

		# Compile shader
		compile_shader(${VAR_DIRECTORY} ${file})
	endforeach()

	add_dependencies(${VAR_DIRECTORY}-copy-shaders ${VAR_DIRECTORY}-copy-folder)

	add_custom_target(${VAR_DIRECTORY})
	add_dependencies(${VAR_DIRECTORY} ${VAR_DIRECTORY}-copy-shaders)
endfunction()

add_example_directory(floppybirb)
add_example_directory(voxelgame)
