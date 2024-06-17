cmake_minimum_required(VERSION 3.10)
project(CMEP)

message("-- Binary dir is: " ${CMAKE_CURRENT_BINARY_DIR})
	
if(NOT MSVC)
	if(NOT APPLE)
		message("-- Will compile luajit for unix (non-osx)")
		#add_custom_target(lua_build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/build_lua.sh linux)
		add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.so ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.a)	

		# Build luajit
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.so ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.a
						   COMMAND MACOSX_DEPLOYMENT_TARGET=${CMEP_MACOS_PREFERRED_TARGET} make
						   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)
	
		# Copy luajit to build dir
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.so ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.a
						   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.so ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.a
						   COMMAND ${CMAKE_COMMAND} -E
						   copy ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.so ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.a ${CMAKE_CURRENT_BINARY_DIR}/../build)
	
	else()
		add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.dylib ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.a)	

		# Build luajit
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dylib ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.a
						   COMMAND make
						   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)

		# Copy luajit to build dir
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.dylib ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.a
						   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dylib ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.a
						   COMMAND ${CMAKE_COMMAND} -E
						   copy ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dylib ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.a ${CMAKE_CURRENT_BINARY_DIR}/../build)

		message("-- Will build lua for MacOS")
		#add_custom_target(lua_build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/build_lua.sh macosx)
	endif()
else()
	message("-- Will compile luajit (Windows); Workdir is: " ${CMAKE_CURRENT_SOURCE_DIR})

	add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.lib)

	# Build luajit
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.lib
					   COMMAND msvcbuild.bat
					   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)

	# Copy luajit to build dir
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.lib
					   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.lib
					   COMMAND ${CMAKE_COMMAND} -E
					   copy ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.lib ${CMAKE_CURRENT_BINARY_DIR}/../build)

endif()