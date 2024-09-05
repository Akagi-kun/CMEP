set(SRC_FILES
	src/feature_test.cpp

	src/Engine.cpp
	src/dllmain.cpp
	src/Exception.cpp
	src/OpaqueEngine.cpp
	src/InternalEngineObject.cpp
	
	src/EnumStringConvertor.cpp

	src/Scene.cpp
	src/Object.cpp
	src/SceneLoader.cpp
	src/SceneManager.cpp

	src/Factories/FontFactory.cpp
	src/Factories/ObjectFactory.cpp
	src/Factories/TextureFactory.cpp

	src/Assets/Font.cpp
	src/Assets/Texture.cpp
	src/Assets/AssetManager.cpp

	src/Rendering/Renderers/Renderer.cpp
	
	src/Rendering/MeshBuilders/IMeshBuilder.cpp
	src/Rendering/MeshBuilders/AxisMeshBuilder.cpp
	src/Rendering/MeshBuilders/TextMeshBuilder.cpp
	src/Rendering/MeshBuilders/SpriteMeshBuilder.cpp
	src/Rendering/MeshBuilders/GeneratorMeshBuilder.cpp
	
	src/Scripting/FFI_Mappings.cpp
	src/Scripting/CrossStateFunctionCall.cpp

	src/Scripting/ILuaScript.cpp
	src/Scripting/EventLuaScript.cpp
	src/Scripting/GeneratorLuaScript.cpp
	
	src/Scripting/Utility.cpp
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
		glfw3
		lua51
		)
elseif(APPLE)
	set(STATIC_LIBRARIES
		glfw
		luajit
		)
else()
	set(STATIC_LIBRARIES
		glfw
		luajit
		)
endif()

add_library(EngineCore SHARED ${SRC_FILES})

target_compile_features(EngineCore PUBLIC cxx_std_20)
set_target_properties(EngineCore PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_compile_options(EngineCore PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_precompile_headers(EngineCore PRIVATE [["vulkan/vulkan.hpp"]] [["glm/glm.hpp"]] [["EnumStringConvertor.hpp"]] [["lua.hpp"]])

target_link_directories(EngineCore PRIVATE ../build ${ENGINE_VULKAN_LINK_PATH})
target_include_directories(EngineCore PRIVATE include
											 ../EngineLogging/export_include
											 ../EngineRenderingVulkan/export_include
											 ../external/luajit/src
											 ../external/glfw/include
											 ../external/nlohmann-json/single_include
											 ../external/VulkanMemoryAllocator/include
											 ../external/lodepng
											 ../external/glm
											 ../common_include
											 ${Vulkan_INCLUDE_DIRS}
											 )

target_link_libraries(EngineCore EngineLogging EngineRenderingVulkan ${STATIC_LIBRARIES} Vulkan::Vulkan)
