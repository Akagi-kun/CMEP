cmake_minimum_required(VERSION 3.10)
project(EngineCore CXX)

set(SRC_FILES
	src/dllmain.cpp
	src/InternalEngineObject.cpp
	src/Engine.cpp
	src/OpaqueEngine.cpp
	src/Object.cpp
	src/Scene.cpp
	src/SceneManager.cpp
	src/SceneLoader.cpp

	src/Factories/TextureFactory.cpp
	src/Factories/ObjectFactory.cpp
	src/Factories/FontFactory.cpp

	src/Logging/ConsoleColors.cpp
	src/Logging/Logging.cpp

	src/Assets/AssetManager.cpp
	src/Assets/Font.cpp
	src/Assets/Mesh.cpp
	src/Assets/Texture.cpp

	src/Rendering/AxisRenderer.cpp
	src/Rendering/MeshRenderer.cpp
	src/Rendering/SpriteRenderer.cpp
	src/Rendering/TextRenderer.cpp

	src/Rendering/SpriteMeshBuilder.cpp
	src/Rendering/TextMeshBuilder.cpp
	
	src/Rendering/Vulkan/VulkanDeviceManager.cpp
	src/Rendering/Vulkan/VulkanRenderingEngine.cpp
	src/Rendering/Vulkan/VulkanRenderingEngine_Init.cpp
	src/Rendering/Vulkan/VulkanImageFactory.cpp
	
	src/Scripting/LuaScript.cpp
	src/Scripting/LuaScriptExecutor.cpp
	src/Scripting/Mappings.cpp
	
	src/Scripting/API/LuaFactories.cpp
	src/Scripting/API/Engine_API.cpp
	src/Scripting/API/Object_API.cpp
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

#include_directories(include)

add_library(EngineCore SHARED ${SRC_FILES})

if(MSVC)
# TODO: Remove /wd4996 after deprecation fixes
	target_compile_options(EngineCore PRIVATE /W3 /WX /wd4996 /wd4068)
else()
	target_compile_options(EngineCore PRIVATE -Wall -Wno-unknown-pragmas -Wno-nullability -Wno-nullability-completeness) # -Wextra -Wpedantic -Werror
endif()

target_link_directories(EngineCore PUBLIC lib ../build) # ../build/Debug 

if((WIN32 OR UNIX) AND NOT APPLE)
	target_link_directories(EngineCore PUBLIC ../external/lua-prebuilt)
elseif(APPLE)
	target_link_directories(EngineCore PUBLIC ../external/lua-5.4.6/src)
endif()

target_include_directories(EngineCore PUBLIC include
											 ../external/luajit/src
											 ../external/glfw/include
											 ../external/nlohmann-json/single_include
											 ../external/VulkanMemoryAllocator/include
											 ../external/lua-prebuilt/include
											 ../external/lodepng
											 ../external/glm
											 ../common_include
											 )
if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Lib)
		target_include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/Include)
	else()
		if(APPLE)
			target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/lib)
			target_include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/include)
		else()
			target_link_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/lib)
			target_include_directories(EngineCore PUBLIC $ENV{VULKAN_SDK}/include)
		endif()
	endif()
else()
	message("Warning! Vulkan SDK environment variable not found!")
endif()

target_link_libraries(EngineCore ${STATIC_LIBRARIES})
