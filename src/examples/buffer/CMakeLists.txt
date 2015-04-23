# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_BUFFER_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_buffer" FILES ${TEST_BUFFER_CPP_SRCS})

SET(testBufferTargetName test_buffer)

add_executable(${testBufferTargetName} ${TEST_BUFFER_CPP_SRCS})

target_link_libraries(${testBufferTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testBufferTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testBufferTargetName})
SET_TESTS_PROPERTIES(${testBufferTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testBufferTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testBufferTargetName}D")
set_target_properties(${testBufferTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testBufferTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testBufferTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testBufferTargetName}
)
