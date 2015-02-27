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
        common/dataExchange.c
        common/dataPack.c
        common/downloadStream.c
        client/verbose.c
        client/platform_id.c
        client/device_id.c
        client/context.c
        client/ocland.c
        client/ocland_icd.c
        client/shortcut.c
    )

    # ===================================================== #
    # Client target                                         #
    # ===================================================== #
    SOURCE_GROUP("ocland_client" FILES ${client_CPP_SRCS})

    SET(clientTagetName ocland)

    ADD_LIBRARY(${clientTagetName} SHARED ${client_CPP_SRCS})

    TARGET_LINK_LIBRARIES(${clientTagetName} ${DEP_LIBS})

    IF(OCLAND_CLIENT_VERBOSE)
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES COMPILE_DEFINITIONS "DATA_EXCHANGE_VERBOSE")
    ENDIF(OCLAND_CLIENT_VERBOSE)
    # ===================================================== #
    # Install client                                        #
    # ===================================================== #
    IF(MSVC)
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES DEBUG_OUTPUT_NAME "${clientTagetName}D")
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
        # dirty hack to avoid Debug/Release subdirectory
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES PREFIX "../")
    ELSEIF(MINGW)
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES DEBUG_OUTPUT_NAME "${clientTagetName}D")
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    ELSE(MSVC)
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
    ENDIF(MSVC)

    IF(WIN32)
        INSTALL(TARGETS ${clientTagetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ELSE(WIN32)
        INSTALL(TARGETS ${clientTagetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ENDIF(WIN32)

    IF(OCLAND_CLIENT_ICD)
        # ===================================================== #
        # Configure ICD vendor file
        # ===================================================== #
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/client/ocland.icd.in
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
        common/dataExchange.c
        common/dataPack.c
        server/dispatcher.c
        server/log.c
        server/ocland.c
        server/ocland_cl.c
        server/ocland_context.c
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

    ADD_EXECUTABLE(${serverTargetName} ${server_CPP_SRCS})

    TARGET_LINK_LIBRARIES(${serverTargetName} ${DEP_LIBS})

    IF(OCLAND_SERVER_VERBOSE)
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES COMPILE_DEFINITIONS "DATA_EXCHANGE_VERBOSE")
    ENDIF(OCLAND_SERVER_VERBOSE)

    # ===================================================== #
    # Install App                                           #
    # ===================================================== #
    IF(MSVC)
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${serverTargetName}D")
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
        # dirty hack to avoid Debug/Release subdirectory
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES PREFIX "../")
    ELSEIF(MINGW)
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${serverTargetName}D")
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    ELSE(MSVC)
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
    ENDIF(MSVC)

    IF(WIN32)
        INSTALL(TARGETS ${serverTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ELSE(WIN32)
        INSTALL(TARGETS ${serverTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ENDIF(WIN32)

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
