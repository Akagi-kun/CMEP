set(SRC_FILES
	src/Exception.cpp
	)

add_library(EngineBase STATIC ${SRC_FILES})

target_compile_features(EngineBase PUBLIC cxx_std_20)
set_target_properties(EngineBase PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_compile_options(EngineBase PRIVATE ${ENGINE_COMPILE_OPTIONS})

target_include_directories(EngineBase PUBLIC include ../common_include)