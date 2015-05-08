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

IF(WIN32)
    SET(NETWORKING_LIBRARY Ws2_32.lib)
ENDIF(WIN32)

IF(OCLAND_CLIENT)
    # ===================================================== #
    # Link
    # ===================================================== #
    SET(DEP_LIBS
        ${NETWORKING_LIBRARY}
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
        common/uploadStream.c
        common/verbose.c
        client/platform_id.c
        client/device_id.c
        client/context.c
        client/command_queue.c
        client/mem.c
        client/sampler.c
        client/program.c
        client/kernel.c
        client/event.c
        client/ocland.c
        client/ocland_icd.c
    )
    IF(WIN32)
        LIST(APPEND client_CPP_SRCS client/exported_functions.def)
    ENDIF(WIN32)


    # ===================================================== #
    # Client target                                         #
    # ===================================================== #
    SOURCE_GROUP("ocland_client" FILES ${client_CPP_SRCS})

    SET(clientTagetName ocland)

    ADD_LIBRARY(${clientTagetName} SHARED ${client_CPP_SRCS})

    TARGET_LINK_LIBRARIES(${clientTagetName} ${DEP_LIBS})

    IF(OCLAND_CLIENT_VERBOSE)
        SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES COMPILE_DEFINITIONS "OCLAND_VERBOSE")
    ENDIF(OCLAND_CLIENT_VERBOSE)
    # ===================================================== #
    # Install client                                        #
    # ===================================================== #
    SET_TARGET_PROPERTIES(${clientTagetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

    IF(WIN32)
        INSTALL(TARGETS ${clientTagetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
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
        IF(WIN32)
            # Registry is set up for ICD in NSIS installer
        ELSE()
            INSTALL(
                FILES
                    ${CMAKE_CURRENT_SOURCE_DIR}/client/ocland.icd
                DESTINATION
                    /etc/OpenCL/vendors
            )
        ENDIF()
    ENDIF(OCLAND_CLIENT_ICD)

ENDIF(OCLAND_CLIENT)

IF(OCLAND_SERVER)
    # ===================================================== #
    # Link
    # ===================================================== #
    SET(DEP_LIBS
        ${NETWORKING_LIBRARY}
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
        common/downloadStream.c
        common/uploadStream.c
        common/verbose.c
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
        SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES COMPILE_DEFINITIONS "OCLAND_VERBOSE")
    ENDIF(OCLAND_SERVER_VERBOSE)

    # ===================================================== #
    # Install App                                           #
    # ===================================================== #
    SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${serverTargetName}D")
    SET_TARGET_PROPERTIES(${serverTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

    INSTALL(TARGETS ${serverTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

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
        IF(UNIX)
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
        ENDIF()
    ENDIF(OCLAND_SERVER_DAEMON)

ENDIF(OCLAND_SERVER)

IF(OCLAND_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF(OCLAND_EXAMPLES)
