# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_FLUSH_FINISH_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_flush_finish" FILES ${TEST_FLUSH_FINISH_CPP_SRCS})

SET(testFlushFinishTargetName test_flush_finish)

add_executable(${testFlushFinishTargetName} ${TEST_FLUSH_FINISH_CPP_SRCS})

target_link_libraries(${testFlushFinishTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testFlushFinishTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testFlushFinishTargetName})
SET_TESTS_PROPERTIES(${testFlushFinishTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testFlushFinishTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testFlushFinishTargetName}D")
set_target_properties(${testFlushFinishTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testFlushFinishTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testFlushFinishTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testFlushFinishTargetName}
)
