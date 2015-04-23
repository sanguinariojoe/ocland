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
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_devices" FILES ${TEST_DEVICES_CPP_SRCS})

SET(testDevicesTargetName test_devices)

add_executable(${testDevicesTargetName} ${TEST_DEVICES_CPP_SRCS})

target_link_libraries(${testDevicesTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testDevicesTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testDevicesTargetName})
SET_TESTS_PROPERTIES(${testDevicesTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testDevicesTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testDevicesTargetName}D")
set_target_properties(${testDevicesTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testDevicesTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testDevicesTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testDevicesTargetName}
)

