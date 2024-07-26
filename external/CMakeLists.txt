cmake_minimum_required(VERSION 3.10)
project(CMEP)

message("-- Binary dir is: " ${CMAKE_CURRENT_BINARY_DIR})

set(VAR_OSX_DEPLOYMENT_TARGET 10.14)
if(NOT MSVC)
	
	set(VAR_LUAJIT_NAME libluajit)
	set(VAR_STATICLIB_EXTENSION a)
	
	if(NOT APPLE) # Unix-like
		set(VAR_DYLIB_EXTENSION so)
	else() # Apple
		set(VAR_DYLIB_EXTENSION so)
	endif()

else() # Windows

	set(VAR_LUAJIT_NAME lua51)
	set(VAR_DYLIB_EXTENSION dll)
	set(VAR_STATICLIB_EXTENSION lib)

endif()

if(NOT MSVC)
	if(NOT APPLE)
		message("-- Will compile luajit for unix (non-osx)")
		add_custom_target(lua_build DEPENDS
						  ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.${VAR_DYLIB_EXTENSION}
						  ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.${VAR_STATICLIB_EXTENSION}
						  ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit-5.1.${VAR_DYLIB_EXTENSION}.2)	

		# Build luajit
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.${VAR_STATICLIB_EXTENSION}
						   COMMAND make
						   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)
	
		# Create necessary linux symlink
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit-5.1.${VAR_DYLIB_EXTENSION}.2
				   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.${VAR_DYLIB_EXTENSION}
				   COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.so ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit-5.1.so.2)
	else()
		message("-- Will build luajit for MacOS (10.14 deploy target)")

		add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/../build/libluajit.${VAR_STATICLIB_EXTENSION})	

		# Build luajit
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/libluajit.${VAR_STATICLIB_EXTENSION}
						   COMMAND MACOSX_DEPLOYMENT_TARGET=${VAR_OSX_DEPLOYMENT_TARGET} make
						   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)
	endif()

else()
	message("-- Will compile luajit (Windows); Workdir is: " ${CMAKE_CURRENT_SOURCE_DIR})

	add_custom_target(lua_build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION})

	# Build luajit
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
					   COMMAND msvcbuild.bat
					   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)

endif()

# Copy luajit to build dir
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
COMMAND ${CMAKE_COMMAND} -E
copy ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION} ${CMAKE_CURRENT_BINARY_DIR}/../build)
	