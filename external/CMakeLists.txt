cmake_minimum_required(VERSION 3.10)
project(CMEP)

message("-- Binary dir is: " ${CMAKE_CURRENT_BINARY_DIR})
message("-- Source dir is: " ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT MSVC) # Unix-like
	message("-- Will compile luajit for unix-like")

	set(VAR_LUAJIT_NAME libluajit)
	set(VAR_STATICLIB_EXTENSION a)
	set(VAR_DYLIB_EXTENSION so)
	
	if(NOT APPLE)
		set(VAR_BUILD_COMMAND make)
	else() # MacOS
		set(VAR_OSX_DEPLOYMENT_TARGET 10.14)
		set(VAR_BUILD_COMMAND MACOSX_DEPLOYMENT_TARGET=${VAR_OSX_DEPLOYMENT_TARGET} make)
	endif()

else() # Windows
	message("-- Will compile luajit for Windows")

	set(VAR_LUAJIT_NAME lua51)
	set(VAR_DYLIB_EXTENSION dll)
	set(VAR_STATICLIB_EXTENSION lib)

	set(VAR_BUILD_COMMAND msvcbuild.bat)

endif()

if(NOT MSVC)
	if(NOT APPLE)
		add_custom_target(lua_build DEPENDS
						  ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
						  ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
						  ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}-5.1.${VAR_DYLIB_EXTENSION}.2)	

		# Create necessary linux symlink
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}-5.1.${VAR_DYLIB_EXTENSION}.2
				   		   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
						   COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.so ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}-5.1.${VAR_DYLIB_EXTENSION}.2)
	else()
	add_custom_target(lua_build DEPENDS
	${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
	${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION})
	endif()

else()
	add_custom_target(lua_build DEPENDS
	${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
	${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION})
endif()

add_custom_command(OUTPUT
				   ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
				   ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
				   COMMAND ${VAR_BUILD_COMMAND}
				   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)

# Copy luajit to build dir
add_custom_command(OUTPUT
				   ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
				   ${CMAKE_CURRENT_BINARY_DIR}/../build/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
				   DEPENDS
				   ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
				   ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
				   COMMAND ${CMAKE_COMMAND} -E
				   copy
				   ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_DYLIB_EXTENSION}
				   ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/${VAR_LUAJIT_NAME}.${VAR_STATICLIB_EXTENSION}
				   ${CMAKE_CURRENT_BINARY_DIR}/../build)
	