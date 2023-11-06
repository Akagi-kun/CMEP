cmake_minimum_required(VERSION 3.5)
project(CMEP CXX)

set(SRC_FILES
	src/main.cpp
	src/test_vulkan.cpp
	../EngineCore/src/Rendering/VulkanRenderingEngine.cpp
	../EngineCore/src/Logging/Logging.cpp
	../EngineCore/src/Logging/ConsoleColors.cpp
	../EngineCore/src/AssetManager.cpp
	../EngineCore/src/Engine.cpp
	../EngineCore/src/GlobalSceneManager.cpp
	../EngineCore/src/ObjectFactory.cpp
	../EngineCore/src/Rendering/Font.cpp
	../EngineCore/src/Rendering/AxisRenderer.cpp
	../EngineCore/src/Rendering/Mesh.cpp
	../EngineCore/src/Rendering/MeshRenderer.cpp
	../EngineCore/src/Rendering/Shader.cpp
	../EngineCore/src/Rendering/SpriteRenderer.cpp
	../EngineCore/src/Rendering/TextRenderer.cpp
	../EngineCore/src/Rendering/Texture.cpp
	../EngineCore/src/Scripting/LuaScript.cpp
	../EngineCore/src/Scripting/LuaScriptExecutor.cpp
	../EngineCore/src/Scripting/Mappings.cpp
	../external/lodepng/lodepng.cpp
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

target_include_directories(runtest PUBLIC ../external ../external/lua-prebuilt/include include ../common_include ../EngineCore/include)
target_link_directories(runtest PUBLIC lib ../build/Debug ../build)
if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		target_link_directories(runtest PUBLIC $ENV{VULKAN_SDK}/Lib ../external/lua-prebuilt)
		target_include_directories(runtest PUBLIC $ENV{VULKAN_SDK}/Include)
	else()
		if(APPLE)
			target_link_directories(runtest PUBLIC $ENV{VULKAN_SDK}/lib ../external/lua-5.4.6/src)
			target_include_directories(runtest PUBLIC $ENV{VULKAN_SDK}/include)
		else()
			target_link_directories(runtest PUBLIC $ENV{VULKAN_SDK}/lib ../external/lua-prebuilt)
			target_include_directories(runtest PUBLIC $ENV{VULKAN_SDK}/include)
		endif()
	endif()
else()
	target_link_directories(runtest PUBLIC ../external/lua-prebuilt)
	message("Warning! Vulkan SDK environment variable not found!")
endif()

target_link_libraries(runtest ${STATIC_LIBRARIES})