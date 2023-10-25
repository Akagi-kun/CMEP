cmake_minimum_required(VERSION 3.0)
project(rungame)

set(SRC_FILES
	main.cpp
	)

add_executable(rungame ${SRC_FILES})

target_link_libraries(rungame EngineCore)