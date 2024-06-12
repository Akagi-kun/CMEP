cmake_minimum_required(VERSION 3.0)
project(examples)


if(MSVC)
    add_custom_target(floppybirb-compile-shaders compile.bat WORKING_DIRECTORY floppybirb\\shaders\\vulkan)
    #add_custom_target(floppybirb-perform-copy xcopy /s /e /i /y examples\\floppybirb build\\game WORKING_DIRECTORY ..)
else()
    add_custom_target(floppybirb-compile-shaders ./compile.sh WORKING_DIRECTORY floppybirb/shaders/vulkan)
    #add_custom_target(floppybirb-perform-copy cp -r examples/floppybirb build COMMAND mv build/floppybirb build/game WORKING_DIRECTORY ..)
endif()	

add_custom_target(floppybirb-perform-copy)
add_custom_command(TARGET floppybirb-perform-copy
                    COMMAND ${CMAKE_COMMAND} -E
                    copy_directory_if_different examples/floppybirb build/game
                    WORKING_DIRECTORY ..
                    )

add_custom_target(floppybirb-remove-excess)
add_custom_command(TARGET floppybirb-remove-excess
                   COMMAND ${CMAKE_COMMAND} -E
                   rm
                   shaders/vulkan/compile.bat
                   shaders/vulkan/cleanup.bat
                   shaders/vulkan/compile.sh
                   shaders/vulkan/cleanup.sh
                   #shaders/vulkan/*.frag
                   WORKING_DIRECTORY ../build/game/
                   )

add_custom_target(floppybirb)
add_dependencies(floppybirb floppybirb-compile-shaders floppybirb-perform-copy floppybirb-remove-excess)
