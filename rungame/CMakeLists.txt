cmake_minimum_required(VERSION 3.12)
project(CMEP CXX)

set(SRC_FILES
	src/main.cpp
	)


if(WIN32)
	set(STATIC_LIBRARIES
		glfw3
		)
elseif(APPLE)
	set(STATIC_LIBRARIES
		glfw
		)
else()
	set(STATIC_LIBRARIES
		glfw
		)
endif()
list(APPEND STATIC_LIBRARIES EngineLogging)

add_executable(rungame ${SRC_FILES})

target_compile_features(rungame PUBLIC cxx_std_17)
set_target_properties(rungame PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(rungame PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(rungame PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_link_directories(rungame PUBLIC ../build)

target_include_directories(rungame PUBLIC include
										  ../EngineLogging/export_include
										  ../EngineCore/export_include
										  ../external/luajit/src
										  ../external/glfw/include
										  ../external/nlohmann-json/single_include
										  ../external/glm
										  ../common_include
										  ${Vulkan_INCLUDE_DIRS}
										  )

#if(DEFINED ENV{VULKAN_SDK})
#	if(WIN32)
#		target_link_directories(rungame PUBLIC $ENV{VULKAN_SDK}/Lib)
#		target_include_directories(rungame PUBLIC $ENV{VULKAN_SDK}/Include)
#	else()
#		target_link_directories(rungame PUBLIC $ENV{VULKAN_SDK}/lib)
#		target_include_directories(rungame PUBLIC $ENV{VULKAN_SDK}/include)
#	endif()
#else()
#	message("Warning! Vulkan SDK environment variable not found!")
#endif()

target_link_libraries(rungame EngineCore ${STATIC_LIBRARIES} Vulkan::Vulkan)