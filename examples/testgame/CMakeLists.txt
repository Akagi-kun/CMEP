cmake_minimum_required(VERSION 3.10)
project(testgame)

if(MSVC)
    add_custom_target(testgame-compile-shaders compile.bat WORKING_DIRECTORY shaders/vulkan)
else()
    add_custom_target(testgame-compile-shaders ./compile.sh WORKING_DIRECTORY shaders/vulkan)
endif()	

add_custom_target(testgame-perform-copy)
add_custom_command(TARGET testgame-perform-copy
                    COMMAND ${CMAKE_COMMAND} -E
                    copy_directory_if_different examples/testgame build/game
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    )

add_custom_command(TARGET testgame-perform-copy
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E
                   rm
                   shaders/vulkan/compile.bat
                   shaders/vulkan/cleanup.bat
                   shaders/vulkan/compile.sh
                   shaders/vulkan/cleanup.sh
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/game
                   )

add_custom_target(testgame)
add_dependencies(testgame testgame-compile-shaders testgame-perform-copy)
