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

link_directories(C:/VulkanSDK/1.3.261.1/Lib ./lib/)

add_library(EngineCore SHARED ${SRC_FILES} ${HEADER_FILES})

target_include_directories(EngineCore PUBLIC include C:/VulkanSDK/1.3.261.1/Include)

target_link_libraries(EngineCore ${STATIC_LIBRARIES})
