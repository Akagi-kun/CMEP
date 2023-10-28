cmake_minimum_required(VERSION 3.0)
project(rungame)

set(SRC_FILES
	main.cpp
	)

set(STATIC_LIBRARIES
	glfw3
	lua54
	)

link_directories(lib)

add_executable(rungame ${SRC_FILES})

target_link_libraries(rungame EngineCore)

target_link_libraries(rungame ${STATIC_LIBRARIES})