cmake_minimum_required(VERSION 3.5)
project(CMEP CXX)

set(SRC_FILES
	src/main.cpp
	src/test_vulkan.cpp
	src/test_logging.cpp
	src/test_engine.cpp
	
	../EngineCore/src/AssetManager.cpp
	../EngineCore/src/Engine.cpp
	../EngineCore/src/SceneManager.cpp

	../EngineCore/src/Factories/ObjectFactory.cpp
	../EngineCore/src/Factories/FontFactory.cpp

	../EngineCore/src/Logging/ConsoleColors.cpp
	../EngineCore/src/Logging/Logging.cpp

	../EngineCore/src/Rendering/Font.cpp
	../EngineCore/src/Rendering/AxisRenderer.cpp
	../EngineCore/src/Rendering/Mesh.cpp
	../EngineCore/src/Rendering/MeshRenderer.cpp
	../EngineCore/src/Rendering/SpriteRenderer.cpp
	../EngineCore/src/Rendering/TextRenderer.cpp
	../EngineCore/src/Rendering/Texture.cpp
	
	../EngineCore/src/Rendering/Vulkan/VulkanRenderingEngine.cpp
	
	../EngineCore/src/Scripting/LuaScript.cpp
	../EngineCore/src/Scripting/LuaScriptExecutor.cpp
	../EngineCore/src/Scripting/Mappings.cpp
	
	../EngineCore/src/Scripting/API/LuaFactories.cpp
	../EngineCore/src/Scripting/API/SceneManager_API.cpp
	../EngineCore/src/Scripting/API/Object_API.cpp
	
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


include_directories(../external ../external/lua-prebuilt/include include ../common_include ../EngineCore/include)
link_directories(lib ../build/Debug ../build)
if(DEFINED ENV{VULKAN_SDK})
	if(WIN32)
		link_directories($ENV{VULKAN_SDK}/Lib ../external/lua-prebuilt)
		include_directories($ENV{VULKAN_SDK}/Include)
	else()
		if(APPLE)
			link_directories($ENV{VULKAN_SDK}/lib ../external/lua-5.4.6/src)
			include_directories($ENV{VULKAN_SDK}/include)
		else()
			link_directories($ENV{VULKAN_SDK}/lib ../external/lua-prebuilt)
			include_directories($ENV{VULKAN_SDK}/include)
		endif()
	endif()
else()
	link_directories(../external/lua-prebuilt)
	message("Warning! Vulkan SDK environment variable not found!")
endif()

add_executable(runtest ${SRC_FILES})

if(MSVC)
    add_custom_target(testgame-compile-shaders compile.bat WORKING_DIRECTORY testgame\\shaders\\vulkan)
    add_custom_target(testgame xcopy /s /e /i /y runtest\\testgame build\\game WORKING_DIRECTORY ..)
else()
    add_custom_target(testgame-compile-shaders ./compile.sh WORKING_DIRECTORY testgame/shaders/vulkan)
    add_custom_target(testgame cp -r runtest/testgame build COMMAND mv build/testgame build/game WORKING_DIRECTORY ..)
endif()
add_dependencies(testgame testgame-compile-shaders)

add_dependencies(runtest testgame)

target_link_libraries(runtest ${STATIC_LIBRARIES})