# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_COMMAND_QUEUE_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_command_queue" FILES ${TEST_COMMAND_QUEUE_CPP_SRCS})

SET(testCommandQueueTargetName test_command_queue)

add_executable(${testCommandQueueTargetName} ${TEST_COMMAND_QUEUE_CPP_SRCS})

target_link_libraries(${testCommandQueueTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testCommandQueueTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testCommandQueueTargetName})
SET_TESTS_PROPERTIES(${testCommandQueueTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testCommandQueueTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testCommandQueueTargetName}D")
set_target_properties(${testCommandQueueTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testCommandQueueTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testCommandQueueTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testCommandQueueTargetName}
)
