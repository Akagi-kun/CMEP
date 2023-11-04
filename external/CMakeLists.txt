cmake_minimum_required(VERSION 3.0)
project(CMEP-external)

add_custom_target(install-vulkan-sdk "./install-vulkan-sdk.sh" WORKING_DIRECTORY .)