cmake_minimum_required(VERSION 3.0)
project(EngineCore)

if (MSVC)
    add_compile_options(/std:c++17)
else()
    add_compile_options(-std=c17)
endif()

set(HEADER_FILES
	)

set(SRC_FILES
	src/AssetManager.cpp
	src/dllmain.cpp
	src/Engine.cpp
	src/GlobalSceneManager.cpp
	src/ObjectFactory.cpp
	src/Logging/ConsoleColors.cpp
	src/Logging/Logging.cpp
	src/Rendering/Font.cpp
	src/Rendering/AxisRenderer.cpp
	src/Rendering/Mesh.cpp
	src/Rendering/MeshRenderer.cpp
	src/Rendering/Shader.cpp
	src/Rendering/SpriteRenderer.cpp
	src/Rendering/TextRenderer.cpp
	src/Rendering/Texture.cpp
	src/Rendering/VulkanRenderingEngine.cpp
	src/Rendering/lodepng/lodepng.cpp
	src/Scripting/LuaScript.cpp
	src/Scripting/LuaScriptExecutor.cpp
	src/Scripting/Mappings.cpp
	)
	
set(STATIC_LIBRARIES
	glfw3
	lua54
	)
	
add_library(EngineCore SHARED ${SRC_FILES} ${HEADER_FILES})

if(DEFINED ENV{VULKAN_SDK})
	target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Lib lib)
	target_include_directories(EngineCore PUBLIC include $ENV{VULKAN_SDK}/Include)
else()
	target_link_directories(EngineCore PUBLIC lib)
	target_include_directories(EngineCore PUBLIC include)
	message("Warning! Vulkan SDK environment variable not found!")
endif()

target_link_libraries(EngineCore ${STATIC_LIBRARIES})
