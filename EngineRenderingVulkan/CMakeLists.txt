set(SRC_FILES
	src/lib_impl.cpp
	src/backend/DebugCallback.cpp

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
	src/objects/Sampler.cpp
	src/objects/CommandPool.cpp
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

add_library(EngineRenderingVulkan STATIC ${SRC_FILES})

target_compile_features(EngineRenderingVulkan PUBLIC cxx_std_20)
set_target_properties(EngineRenderingVulkan PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(EngineRenderingVulkan PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_CONSTRUCTORS)
target_compile_options(EngineRenderingVulkan PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_precompile_headers(EngineRenderingVulkan PRIVATE [["vulkan/vulkan.hpp"]] [["vulkan/vulkan_raii.hpp"]])

target_link_directories(EngineRenderingVulkan PRIVATE ../build)
target_include_directories(EngineRenderingVulkan PRIVATE
											 ../EngineBase/export_include
											 ../EngineLogging/export_include
											 ../external/glfw/include
											 ../external/VulkanMemoryAllocator/include
											 ../external/lodepng
											 ../external/glm
											 ../common_include
											 ${Vulkan_INCLUDE_DIRS}
											 )
target_include_directories(EngineRenderingVulkan PUBLIC include)

target_link_libraries(EngineRenderingVulkan EngineBase EngineLogging ${STATIC_LIBRARIES} Vulkan::Vulkan)
