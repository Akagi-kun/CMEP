cmake_minimum_required(VERSION 3.10)
project(EngineLogging CXX)

set(SRC_FILES
	src/Logging.cpp
	src/ConsoleColors.cpp
	)

add_library(EngineLogging STATIC ${SRC_FILES})

target_compile_options(EngineLogging PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_link_directories(EngineLogging PUBLIC ../build)

target_include_directories(EngineLogging PUBLIC include
											 ../external/glm
											 ../common_include
											 )
