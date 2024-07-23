cmake_minimum_required(VERSION 3.10)
project(floppybirb)

if(MSVC)
    add_custom_target(floppybirb-compile-shaders compile.bat WORKING_DIRECTORY shaders/vulkan)
else()
    add_custom_target(floppybirb-compile-shaders ./compile.sh WORKING_DIRECTORY shaders/vulkan)
endif()	

add_custom_target(floppybirb-perform-copy)
add_custom_command(TARGET floppybirb-perform-copy
                    COMMAND ${CMAKE_COMMAND} -E
                    copy_directory_if_different examples/floppybirb build/game
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    )

add_custom_command(TARGET floppybirb-perform-copy
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E
                   rm
                   shaders/vulkan/compile.bat
                   shaders/vulkan/cleanup.bat
                   shaders/vulkan/compile.sh
                   shaders/vulkan/cleanup.sh
                   CMakeLists.txt
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/game
                   )

add_custom_target(floppybirb)
add_dependencies(floppybirb floppybirb-compile-shaders floppybirb-perform-copy)
