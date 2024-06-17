cmake_minimum_required(VERSION 3.5)
project(CMEP)

message("-- Binary dir is: " ${CMAKE_CURRENT_BINARY_DIR})
	
if(NOT MSVC)
	if(NOT APPLE)
		message("-- Will compile luajit for unix (non-osx)")
		#add_custom_target(lua_build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/build_lua.sh linux)
		add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.so)	

		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.so
						   COMMAND make
						   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)
	
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.so
						   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.so
						   COMMAND ${CMAKE_COMMAND} -E
						   copy ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.so build/lua51.so
						   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
	
	else()
		message("-- Will build lua for MacOS")
		add_custom_target(lua_build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/build_lua.sh macosx)
	endif()
else()
	#message("-- Will copy lua from prebuilt (Windows); Workdir is: " ${CMAKE_CURRENT_SOURCE_DIR})
	message("-- Will compile luajit (Windows); Workdir is: " ${CMAKE_CURRENT_SOURCE_DIR})

	add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.lib)

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.lib
					   COMMAND msvcbuild.bat
					   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/../build/lua51.lib
					   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.lib
					   COMMAND ${CMAKE_COMMAND} -E
					   copy ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.dll ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/lua51.lib build
					   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)

	#add_custom_command(TARGET lua_build
	#					COMMAND ${CMAKE_COMMAND} -E
	#					copy external/lua-prebuilt/lua54.dll build/lua54.dll
	#					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
	#					)
endif()