# ===================================================== #
# Destination
# ===================================================== #
SET(CMAKE_INSTALL_BINDIR bin)
SET(CMAKE_INSTALL_LIBDIR lib/ocland)

# ===================================================== #
# Include
# ===================================================== #
INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}/include
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
	${OPENCL_INCLUDE_DIRS}
	${THREADS_INCLUDE_PATH}
)

IF(OCLAND_CLIENT)
	# ===================================================== #
	# Link
	# ===================================================== #
	SET(DEP_LIBS 
		${THREADS_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)

	# ===================================================== #
	# Sources to compile client                             #
	# ===================================================== #
	SET(client_CPP_SRCS
		common/dataExchange.c
		client/ocland.c
		client/ocland_icd.c
		client/shortcut.c
	)

	# ===================================================== #
	# Client target                                         #
	# ===================================================== #
	SOURCE_GROUP("ocland_client" FILES ${client_CPP_SRCS})

	SET(clientTagetName ocland)

	add_library(${clientTagetName} SHARED ${client_CPP_SRCS})

	target_link_libraries(${clientTagetName} ${DEP_LIBS})

	# ===================================================== #
	# Install client                                        #
	# ===================================================== #
	if(MSVC)
	    set_target_properties(${clientTagetName} PROPERTIES DEBUG_OUTPUT_NAME "${clientTagetName}D")
	    set_target_properties(${clientTagetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	    # dirty hack to avoid Debug/Release subdirectory
	    set_target_properties(${clientTagetName} PROPERTIES PREFIX "../")
	elseif(MINGW)
	    set_target_properties(${clientTagetName} PROPERTIES DEBUG_OUTPUT_NAME "${clientTagetName}D")
	    set_target_properties(${clientTagetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	else(MSVC)
	    set_target_properties(${clientTagetName} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
	    set_target_properties(${clientTagetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
	endif(MSVC)

	if(WIN32)
	    INSTALL(TARGETS ${clientTagetName}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    )
	else(WIN32)
	    INSTALL(TARGETS ${clientTagetName}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    )
	endif(WIN32)

	IF(OCLAND_CLIENT_ICD)
		# ===================================================== #
		# Configure ICD vendor file
		# ===================================================== #
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/client/ocland.icd.in
		${CMAKE_CURRENT_SOURCE_DIR}/client/ocland.icd @ONLY)

		# ===================================================== #
		# Install ICD vendor                                    #
		# ===================================================== #
		INSTALL(
			FILES
				${CMAKE_CURRENT_SOURCE_DIR}/client/ocland.icd
			DESTINATION
				/etc/OpenCL/vendors
		)
	ENDIF(OCLAND_CLIENT_ICD)

ENDIF(OCLAND_CLIENT)

IF(OCLAND_SERVER)
	# ===================================================== #
	# Link
	# ===================================================== #
	SET(DEP_LIBS 
		${OPENCL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)

	# ===================================================== #
	# Sources to compile app                                #
	# ===================================================== #
	SET(server_CPP_SRCS
		common/dataExchange.c
		server/dispatcher.c
		server/log.c
		server/ocland.c
		server/ocland_cl.c
		server/ocland_event.c
		server/ocland_mem.c
		server/validator.c
	)

	# ===================================================== #
	# App target                                         #
	# ===================================================== #
	SOURCE_GROUP("ocland_server" FILES ${server_CPP_SRCS})

	SET(serverTargetName ocland_server)

	add_executable(${serverTargetName} ${server_CPP_SRCS})

	target_link_libraries(${serverTargetName} ${DEP_LIBS})

	# ===================================================== #
	# Install App                                           #
	# ===================================================== #
	if(MSVC)
	    set_target_properties(${serverTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${serverTargetName}D")
	    set_target_properties(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	    # dirty hack to avoid Debug/Release subdirectory
	    set_target_properties(${serverTargetName} PROPERTIES PREFIX "../")
	elseif(MINGW)
	    set_target_properties(${serverTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${serverTargetName}D")
	    set_target_properties(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	else(MSVC)
	    set_target_properties(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	    set_target_properties(${serverTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
	endif(MSVC)

	if(WIN32)
	    INSTALL(TARGETS ${serverTargetName}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    )
	else(WIN32)
	    INSTALL(TARGETS ${serverTargetName}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	    )
	endif(WIN32)
ENDIF(OCLAND_SERVER)

IF(OCLAND_EXAMPLES)
	# ===================================================== #
	# Link
	# ===================================================== #
	SET(DEP_LIBS 
		${OPENCL_LIBRARIES}
	)

	# ===================================================== #
	# Sources to compile app                                #
	# ===================================================== #
	SET(exampleA_CPP_SRCS
		test/main.c
	)

	# ===================================================== #
	# App target                                         #
	# ===================================================== #
	SOURCE_GROUP("test" FILES ${exampleA_CPP_SRCS})

	SET(testTargetName test)

	add_executable(${testTargetName} ${exampleA_CPP_SRCS})

	target_link_libraries(${testTargetName} ${DEP_LIBS})

	# ===================================================== #
	# Install App                                           #
	# ===================================================== #
	if(MSVC)
	    set_target_properties(${testTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testTargetName}D")
	    set_target_properties(${testTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	    # dirty hack to avoid Debug/Release subdirectory
	    set_target_properties(${testTargetName} PROPERTIES PREFIX "../")
	elseif(MINGW)
	    set_target_properties(${testTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testTargetName}D")
	    set_target_properties(${testTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	else(MSVC)
	    set_target_properties(${testTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
	    set_target_properties(${testTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
	endif(MSVC)

	if(WIN32)
	    INSTALL(TARGETS ${testTargetName}
		RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testTargetName}
		LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testTargetName}
	    )
	else(WIN32)
	    INSTALL(TARGETS ${testTargetName}
		RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testTargetName}
		LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testTargetName}
	    )
	endif(WIN32)
ENDIF(OCLAND_EXAMPLES)
