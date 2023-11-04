cmake_minimum_required(VERSION 3.5)
project(rungame CXX)

set(SRC_FILES
	src/main.cpp
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


message(${builddir})
add_executable(rungame ${SRC_FILES})

target_link_directories(rungame PUBLIC ../build/ ../external/lua/)
target_include_directories(rungame PUBLIC include/)

target_link_libraries(rungame EngineCore)

target_link_libraries(rungame ${STATIC_LIBRARIES})