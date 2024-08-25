cmake_minimum_required(VERSION 3.12)
project(EngineCore CXX)

set(SRC_FILES
	src/lib_impl.cpp

	src/backend/Instance.cpp
	src/backend/DeviceScore.cpp
	src/backend/LogicalDevice.cpp
	src/backend/PhysicalDevice.cpp
	src/backend/MemoryAllocator.cpp
	
	src/rendering/Window.cpp
	src/rendering/Surface.cpp
	src/rendering/Pipeline.cpp
	src/rendering/Swapchain.cpp
	src/rendering/RenderPass.cpp
	src/rendering/ShaderModule.cpp
	src/rendering/PipelineManager.cpp

	src/objects/Image.cpp
	src/objects/Buffer.cpp
	src/objects/CommandPool.cpp
	src/objects/SampledImage.cpp
	src/objects/CommandBuffer.cpp
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

add_library(EngineRenderingVulkan STATIC ${SRC_FILES})

target_compile_features(EngineRenderingVulkan PUBLIC cxx_std_17)
set_target_properties(EngineRenderingVulkan PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(EngineRenderingVulkan PRIVATE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
target_compile_options(EngineRenderingVulkan PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_precompile_headers(EngineRenderingVulkan PRIVATE [["ImportVulkan.hpp"]])

target_link_directories(EngineRenderingVulkan PUBLIC ../build ${ENGINE_VULKAN_LINK_PATH})
target_include_directories(EngineRenderingVulkan PUBLIC include
											 ../EngineLogging/export_include
											 ../external/glfw/include
											 ../external/VulkanMemoryAllocator/include
											 ../external/lodepng
											 ../external/glm
											 ../common_include
											 ${Vulkan_INCLUDE_DIRS}
											 )

target_link_libraries(EngineRenderingVulkan ${STATIC_LIBRARIES} Vulkan::Vulkan)
