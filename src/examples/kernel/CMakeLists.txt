# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_KERNEL_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_kernel" FILES ${TEST_KERNEL_CPP_SRCS})

SET(testKernelTargetName test_kernel)

add_executable(${testKernelTargetName} ${TEST_KERNEL_CPP_SRCS})

target_link_libraries(${testKernelTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testKernelTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testKernelTargetName})
SET_TESTS_PROPERTIES(${testKernelTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
set_target_properties(${testKernelTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testKernelTargetName}D")
set_target_properties(${testKernelTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

INSTALL(TARGETS ${testKernelTargetName}
    RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testKernelTargetName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testKernelTargetName}
)

