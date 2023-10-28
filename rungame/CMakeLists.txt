cmake_minimum_required(VERSION 3.0)
project(rungame)

set(SRC_FILES
	main.cpp
	)

set(STATIC_LIBRARIES
	glfw3
	lua54
	)

link_directories(C:/VulkanSDK/1.3.261.1/Lib ./lib/)

add_executable(rungame ${SRC_FILES})

target_link_libraries(rungame EngineCore)

target_link_libraries(rungame ${STATIC_LIBRARIES})