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

IF(OCLAND_USE_COMPRESSION)
	INCLUDE_DIRECTORIES(${COMPRESSION_INCLUDE_DIRS})
ENDIF(OCLAND_USE_COMPRESSION)

IF(UNIX)
	set(DATA_EXCHANGE_SOURCE common/unix/dataExchange.c)
ELSEIF(WIN32)
	set(DATA_EXCHANGE_SOURCE common/windows/dataExchange.c)
ELSE()
	MESSAGE(FATAL_ERROR "No OS-specific networking implementation for current system.")
ENDIF()

IF(OCLAND_CLIENT)
	# ===================================================== #
	# Link
	# ===================================================== #
	SET(DEP_LIBS 
		${THREADS_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)
	IF(OCLAND_USE_COMPRESSION)
		SET(DEP_LIBS 
			${DEP_LIBS}
			${COMPRESSION_LIBRARIES}
		)
	ENDIF(OCLAND_USE_COMPRESSION)

	# ===================================================== #
	# Sources to compile client                             #
	# ===================================================== #
	SET(client_CPP_SRCS
		${DATA_EXCHANGE_SOURCE}
		common/dataPack.c
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
		${THREADS_LIBRARIES}
		${OPENCL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)
	IF(OCLAND_USE_COMPRESSION)
		SET(DEP_LIBS 
			${DEP_LIBS}
			${COMPRESSION_LIBRARIES}
		)
	ENDIF(OCLAND_USE_COMPRESSION)

	# ===================================================== #
	# Sources to compile app                                #
	# ===================================================== #
	SET(server_CPP_SRCS
		${DATA_EXCHANGE_SOURCE}
		common/dataPack.c
		server/dispatcher.c
		server/log.c
		server/ocland.c
		server/ocland_cl.c
		server/ocland_event.c
		server/ocland_mem.c
		server/ocland_version.c
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

	IF(OCLAND_SERVER_DAEMON)
		# ===================================================== #
		# Configure daemon script
		# ===================================================== #
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/server-daemon/ocland.in
		${CMAKE_CURRENT_SOURCE_DIR}/server-daemon/ocland @ONLY)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/server-daemon/ocland.conf.in
		${CMAKE_CURRENT_SOURCE_DIR}/server-daemon/ocland.conf @ONLY)

		# ===================================================== #
		# Install daemon                                        #
		# ===================================================== #
		INSTALL(
			FILES
				${CMAKE_CURRENT_SOURCE_DIR}/server-daemon/ocland
			DESTINATION
				/etc/init.d
    		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                		GROUP_EXECUTE             GROUP_READ
		                WORLD_EXECUTE             WORLD_READ             
		)
		INSTALL(
			FILES
				${CMAKE_CURRENT_SOURCE_DIR}/server-daemon/ocland.conf
			DESTINATION
				/etc/init
		)
	ENDIF(OCLAND_SERVER_DAEMON)

ENDIF(OCLAND_SERVER)

IF(OCLAND_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF(OCLAND_EXAMPLES)
