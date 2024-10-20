add_library(EngineLogging STATIC)

target_sources(EngineLogging PRIVATE
	src/Logging.cpp
	src/LoggingPrefix.cpp
	)

target_compile_features(EngineLogging PUBLIC cxx_std_20)
set_target_properties(EngineLogging PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)

target_compile_options(EngineLogging PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_link_directories(EngineLogging PRIVATE ../build)
target_include_directories(EngineLogging PRIVATE include
											 ../external/glm
											 ../common_include
											 )
