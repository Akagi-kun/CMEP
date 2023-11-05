cmake_minimum_required(VERSION 3.5)
project(CMEP CXX)

if(WIN32)
	set(STATIC_LIBRARIES
		vulkan-1
		glfw3
		lua54
		)
else()
	set(STATIC_LIBRARIES
		vulkan
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

add_executable(test_vulkan src/test_vulkan.cpp ../EngineCore/src/Rendering/VulkanRenderingEngine.cpp ../EngineCore/src/Logging/Logging.cpp ../EngineCore/src/Logging/ConsoleColors.cpp)

target_link_libraries(test_vulkan ${STATIC_LIBRARIES})