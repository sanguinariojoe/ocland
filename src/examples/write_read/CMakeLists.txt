# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_WRITE_READ_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_write_read" FILES ${TEST_WRITE_READ_CPP_SRCS})

SET(testWriteReadTargetName test_write_read)

add_executable(${testWriteReadTargetName} ${TEST_WRITE_READ_CPP_SRCS})

target_link_libraries(${testWriteReadTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testWriteReadTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testWriteReadTargetName})
SET_TESTS_PROPERTIES(${testWriteReadTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testWriteReadTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testWriteReadTargetName}D")
set_target_properties(${testWriteReadTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testWriteReadTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testWriteReadTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testWriteReadTargetName}
)
