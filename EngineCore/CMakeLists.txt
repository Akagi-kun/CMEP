cmake_minimum_required(VERSION 3.12)
project(EngineCore CXX)

set(SRC_FILES
	src/dllmain.cpp
	src/InternalEngineObject.cpp
	src/Engine.cpp
	src/OpaqueEngine.cpp
	
	src/EnumStringConvertor.cpp

	src/Object.cpp
	src/Scene.cpp
	src/SceneManager.cpp
	src/SceneLoader.cpp

	src/Factories/TextureFactory.cpp
	src/Factories/ObjectFactory.cpp
	src/Factories/FontFactory.cpp

	src/Assets/AssetManager.cpp
	src/Assets/Font.cpp
	src/Assets/Texture.cpp

	src/Rendering/Renderers/Renderer.cpp
	
	src/Rendering/MeshBuilders/IMeshBuilder.cpp
	src/Rendering/MeshBuilders/AxisMeshBuilder.cpp
	src/Rendering/MeshBuilders/TextMeshBuilder.cpp
	src/Rendering/MeshBuilders/SpriteMeshBuilder.cpp
	src/Rendering/MeshBuilders/GeneratorMeshBuilder.cpp
	
	src/Rendering/Vulkan/PipelineManager.cpp
	src/Rendering/Vulkan/VmaImplementation.cpp
	
	src/Rendering/Vulkan/Wrappers/LogicalDevice.cpp
	src/Rendering/Vulkan/Wrappers/MemoryAllocator.cpp
	src/Rendering/Vulkan/Wrappers/Instance.cpp
	src/Rendering/Vulkan/Wrappers/Surface.cpp
	src/Rendering/Vulkan/Wrappers/Window.cpp
	src/Rendering/Vulkan/Wrappers/ShaderModule.cpp
	src/Rendering/Vulkan/Wrappers/Pipeline.cpp
	src/Rendering/Vulkan/Wrappers/Image.cpp
	src/Rendering/Vulkan/Wrappers/SampledImage.cpp
	src/Rendering/Vulkan/Wrappers/RenderPass.cpp
	src/Rendering/Vulkan/Wrappers/Swapchain.cpp
	src/Rendering/Vulkan/Wrappers/Buffer.cpp
	src/Rendering/Vulkan/Wrappers/DeviceScore.cpp
	src/Rendering/Vulkan/Wrappers/PhysicalDevice.cpp
	src/Rendering/Vulkan/Wrappers/CommandPool.cpp
	src/Rendering/Vulkan/Wrappers/CommandBuffer.cpp
	
	src/Scripting/CrossStateFunctionCall.cpp
	
	src/Scripting/ILuaScript.cpp
	src/Scripting/EventLuaScript.cpp
	src/Scripting/GeneratorLuaScript.cpp
	
	src/Scripting/Mappings.cpp
	src/Scripting/API/LuaFactories.cpp
	src/Scripting/API/Engine_API.cpp
	src/Scripting/API/Object_API.cpp
	src/Scripting/API/Scene_API.cpp
	src/Scripting/API/SceneManager_API.cpp
	src/Scripting/API/AssetManager_API.cpp

	../external/lodepng/lodepng.cpp
	)

if(WIN32)
	set(STATIC_LIBRARIES
		vulkan-1
		glfw3
		lua51
		)
elseif(APPLE)
	set(STATIC_LIBRARIES
		vulkan
		glfw
		luajit
		)
else()
	set(STATIC_LIBRARIES
		vulkan
		glfw
		luajit
		)
endif()

list(APPEND STATIC_LIBRARIES EngineLogging)

add_library(EngineCore SHARED ${SRC_FILES})

target_compile_features(EngineCore PUBLIC cxx_std_17)
set_target_properties(EngineCore PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(EngineCore PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(EngineCore PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_link_directories(EngineCore PUBLIC ../build)

target_include_directories(EngineCore PUBLIC include
											 ../EngineLogging/export_include
											 ../external/luajit/src
											 ../external/glfw/include
											 ../external/nlohmann-json/single_include
											 ../external/VulkanMemoryAllocator/include
											 ../external/lodepng
											 ../external/glm
											 ../common_include
											 )
if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Lib)
		target_include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Include)
	else()
		target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/lib)
		target_include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/include)
	endif()
else()
	message("Warning! Vulkan SDK environment variable not found!")
endif()

target_link_libraries(EngineCore ${STATIC_LIBRARIES})
