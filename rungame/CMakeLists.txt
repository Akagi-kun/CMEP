cmake_minimum_required(VERSION 3.5)
project(CMEP CXX)

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

include_directories(EngineCore PUBLIC ../external ../external/lua/include include ../common_include ../EngineCore/include)
if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Lib lib ../build/Debug ../build ../external/lua)
		include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/include)
	else()
		link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/lib lib ../build/Debug ../build ../external/lua)
		include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/include)
	endif()
else()
	link_directories(EngineCore PUBLIC lib ../build/Debug ../build ../external/lua)
	message("Warning! Vulkan SDK environment variable not found!")
endif()

add_executable(rungame ${SRC_FILES})

target_link_libraries(rungame EngineCore ${STATIC_LIBRARIES})