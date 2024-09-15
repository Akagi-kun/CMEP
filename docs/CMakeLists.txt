add_custom_target(build_docs
				  COMMAND
				  ${DOXYGEN_EXECUTABLE} docs/Doxyfile
				  WORKING_DIRECTORY
				  ${CMAKE_SOURCE_DIR}
				  )