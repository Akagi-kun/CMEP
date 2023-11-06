cmake_minimum_required(VERSION 3.5)
project(CMEP CXX)

set(SRC_FILES
	src/main.cpp
	src/test_vulkan.cpp
	../EngineCore/src/Rendering/VulkanRenderingEngine.cpp
	../EngineCore/src/Logging/Logging.cpp
	../EngineCore/src/Logging/ConsoleColors.cpp
	)


if(WIN32)
	set(STATIC_LIBRARIES
		vulkan-1
		glfw3
		lua54
		)
elseif(APPLE)
	set(STATIC_LIBRARIES
		vulkan
		glfw
		lua
		)
else()
	set(STATIC_LIBRARIES
		vulkan
		glfw
		lua54
		)
endif()

add_executable(runtest ${SRC_FILES})

include_directories(runtest PUBLIC ../external ../external/lua-prebuilt/include include ../common_include ../EngineCore/include)
if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		target_link_directories(runtest PUBLIC $ENV{VULKAN_SDK}/Lib lib ../build/Debug ../build ../external/lua-prebuilt)
		target_include_directories(runtest PUBLIC $ENV{VULKAN_SDK}/Include)
	else()
		if(APPLE)
				target_link_directories(runtest PUBLIC $ENV{VULKAN_SDK}/lib lib ../build/Debug ../build ../external/lua-5.4.6/src)
			target_include_directories(runtest PUBLIC $ENV{VULKAN_SDK}/include)
		else()
			target_link_directories(runtest PUBLIC $ENV{VULKAN_SDK}/lib lib ../build/Debug ../build ../external/lua-prebuilt)
			target_include_directories(runtest PUBLIC $ENV{VULKAN_SDK}/include)
		endif()
	endif()
else()
	link_directories(runtest PUBLIC lib ../build/Debug ../build ../external/lua)
	message("Warning! Vulkan SDK environment variable not found!")
endif()

target_link_libraries(runtest ${STATIC_LIBRARIES})