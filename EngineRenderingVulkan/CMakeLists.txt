cmake_minimum_required(VERSION 3.12)
project(EngineCore CXX)

set(SRC_FILES
	src/VmaImplementation.cpp
	src/VkDispatchLoaderDynamic.cpp

	src/PipelineManager.cpp
	
	src/Wrappers/LogicalDevice.cpp
	src/Wrappers/MemoryAllocator.cpp
	src/Wrappers/Instance.cpp
	src/Wrappers/Surface.cpp
	src/Wrappers/Window.cpp
	src/Wrappers/ShaderModule.cpp
	src/Wrappers/Pipeline.cpp
	src/Wrappers/Image.cpp
	src/Wrappers/SampledImage.cpp
	src/Wrappers/RenderPass.cpp
	src/Wrappers/Swapchain.cpp
	src/Wrappers/Buffer.cpp
	src/Wrappers/DeviceScore.cpp
	src/Wrappers/PhysicalDevice.cpp
	src/Wrappers/CommandPool.cpp
	src/Wrappers/CommandBuffer.cpp
	)

if(WIN32)
	set(STATIC_LIBRARIES
		vulkan-1
		glfw3
		)
elseif(APPLE)
	set(STATIC_LIBRARIES
		vulkan
		glfw
		)
else()
	set(STATIC_LIBRARIES
		vulkan
		glfw
		)
endif()

list(APPEND STATIC_LIBRARIES EngineLogging)

add_library(EngineRenderingVulkan STATIC ${SRC_FILES})

target_compile_features(EngineRenderingVulkan PUBLIC cxx_std_17)
set_target_properties(EngineRenderingVulkan PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(EngineRenderingVulkan PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(EngineLogging PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(EngineRenderingVulkan PRIVATE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

target_compile_options(EngineRenderingVulkan PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_link_directories(EngineRenderingVulkan PUBLIC ../build ${ENGINE_VULKAN_LINK_PATH})

target_include_directories(EngineRenderingVulkan PUBLIC include
											 ../EngineLogging/export_include
											 ../external/glfw/include
											 ../external/VulkanMemoryAllocator/include
											 ../external/lodepng
											 ../external/glm
											 ../common_include
											 ${ENGINE_VULKAN_INCLUDE_PATH}
											 )

target_link_libraries(EngineRenderingVulkan ${STATIC_LIBRARIES})
