cmake_minimum_required(VERSION 3.5)
project(EngineCore CXX)

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
	src/Scripting/LuaScript.cpp
	src/Scripting/LuaScriptExecutor.cpp
	src/Scripting/Mappings.cpp
	../external/lodepng/lodepng.cpp
	)

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

add_library(EngineCore SHARED ${SRC_FILES})

if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Lib lib ../build/Debug ../build ../external/lua-prebuilt)
		target_include_directories(EngineCore PUBLIC include ../external ../external/lua-prebuilt/include $ENV{VULKAN_SDK}/Include)
	else()
		if(APPLE)
			target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/lib lib ../build/Debug ../build ../external/lua-5.4.6/src)
			target_include_directories(EngineCore PUBLIC include ../external ../external/lua-prebuilt/include $ENV{VULKAN_SDK}/include)
		else()
			target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/lib lib ../build/Debug ../build ../external/lua-prebuilt)
			target_include_directories(EngineCore PUBLIC include ../external ../external/lua-prebuilt/include $ENV{VULKAN_SDK}/include)
		endif()
	endif()
else()
	target_link_directories(EngineCore PUBLIC lib ../build/Debug ../build ../external/lua-prebuilt ../external/lua-)
	target_include_directories(EngineCore PUBLIC ../external ../external/lua-prebuilt/include include)
	message("Warning! Vulkan SDK environment variable not found!")
endif()

target_link_libraries(EngineCore ${STATIC_LIBRARIES})
