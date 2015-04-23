# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_PROGRAM_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_program" FILES ${TEST_PROGRAM_CPP_SRCS})

SET(testProgramTargetName test_program)

add_executable(${testProgramTargetName} ${TEST_PROGRAM_CPP_SRCS})

target_link_libraries(${testProgramTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testProgramTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testProgramTargetName})
SET_TESTS_PROPERTIES(${testProgramTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testProgramTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testProgramTargetName}D")
set_target_properties(${testProgramTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testProgramTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testProgramTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testProgramTargetName}
)

