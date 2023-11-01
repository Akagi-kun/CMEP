cmake_minimum_required(VERSION 3.0)
project(examples)

add_custom_target(floppybirb-compile-shaders compile.bat WORKING_DIRECTORY floppybirb\\shaders\\vulkan)
add_custom_target(floppybirb xcopy /s /e /i /y examples\\floppybirb build\\game WORKING_DIRECTORY ..)
add_dependencies(floppybirb floppybirb-compile-shaders)
