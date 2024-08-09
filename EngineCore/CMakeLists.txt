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

	src/Rendering/IRenderer.cpp
	src/Rendering/Renderer2D.cpp
	src/Rendering/Renderer3D.cpp

	src/Rendering/IMeshBuilder.cpp
	src/Rendering/TextMeshBuilder.cpp
	src/Rendering/AxisMeshBuilder.cpp
	src/Rendering/SpriteMeshBuilder.cpp
	src/Rendering/GeneratorMeshBuilder.cpp
	
	src/Rendering/Vulkan/PipelineManager.cpp

	src/Rendering/Vulkan/VmaImplementation.cpp

	src/Rendering/Vulkan/VDeviceManager.cpp
	src/Rendering/Vulkan/VulkanRenderingEngine.cpp
	src/Rendering/Vulkan/VulkanRenderingEngine_Init.cpp
	
	src/Rendering/Vulkan/Wrappers/VShaderModule.cpp
	src/Rendering/Vulkan/Wrappers/VPipeline.cpp
	src/Rendering/Vulkan/Wrappers/VImage.cpp
	src/Rendering/Vulkan/Wrappers/VSampledImage.cpp
	src/Rendering/Vulkan/Wrappers/RenderPass.cpp
	src/Rendering/Vulkan/Wrappers/VSwapchain.cpp
	src/Rendering/Vulkan/Wrappers/VBuffer.cpp
	src/Rendering/Vulkan/Wrappers/VCommandPool.cpp
	src/Rendering/Vulkan/Wrappers/VCommandBuffer.cpp
	
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
