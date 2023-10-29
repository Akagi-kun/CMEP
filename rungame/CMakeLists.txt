cmake_minimum_required(VERSION 3.0)
project(rungame CXX)

set(SRC_FILES
	main.cpp
	)

if(WIN32)
	set(STATIC_LIBRARIES
		glfw3
		lua54
		)
else()
	set(STATIC_LIBRARIES
		glfw
		lua54
		)
endif()

link_directories(lib)

add_executable(rungame ${SRC_FILES})

target_link_libraries(rungame EngineCore)

target_link_libraries(rungame ${STATIC_LIBRARIES})