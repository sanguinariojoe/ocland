# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS 
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_DEVICES_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                         #
# ===================================================== #
SOURCE_GROUP("test_devices" FILES ${TEST_DEVICES_CPP_SRCS})

SET(testDevicesTargetName test_devices)

add_executable(${testDevicesTargetName} ${TEST_DEVICES_CPP_SRCS})

target_link_libraries(${testDevicesTargetName} ${DEP_LIBS})

# ===================================================== #
# Install App                                           #
# ===================================================== #
if(MSVC)
    set_target_properties(${testDevicesTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testDevicesTargetName}D")
    set_target_properties(${testDevicesTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # dirty hack to avoid Debug/Release subdirectory
    set_target_properties(${testDevicesTargetName} PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(${testDevicesTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testDevicesTargetName}D")
    set_target_properties(${testDevicesTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
else(MSVC)
    set_target_properties(${testDevicesTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testDevicesTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

if(WIN32)
    INSTALL(TARGETS ${testDevicesTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testDevicesTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testDevicesTargetName}
    )
else(WIN32)
    INSTALL(TARGETS ${testDevicesTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testDevicesTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testDevicesTargetName}
    )
endif(WIN32)