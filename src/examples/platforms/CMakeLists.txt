# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_PLATFORMS_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_platforms" FILES ${TEST_PLATFORMS_CPP_SRCS})

SET(testPlatformsTargetName test_platforms)

add_executable(${testPlatformsTargetName} ${TEST_PLATFORMS_CPP_SRCS})

target_link_libraries(${testPlatformsTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testPlatformsTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testPlatformsTargetName})
SET_TESTS_PROPERTIES(${testPlatformsTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #

set_target_properties(${testPlatformsTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testPlatformsTargetName}D")
set_target_properties(${testPlatformsTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testPlatformsTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testPlatformsTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testPlatformsTargetName}
)

