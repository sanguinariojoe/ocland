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
# App target                                         #
# ===================================================== #
SOURCE_GROUP("test_kernel" FILES ${TEST_KERNEL_CPP_SRCS})

SET(testKernelTargetName test_kernel)

add_executable(${testKernelTargetName} ${TEST_KERNEL_CPP_SRCS})

target_link_libraries(${testKernelTargetName} ${DEP_LIBS})

# ===================================================== #
# Install App                                           #
# ===================================================== #
if(MSVC)
    set_target_properties(${testKernelTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testKernelTargetName}D")
    set_target_properties(${testKernelTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # dirty hack to avoid Debug/Release subdirectory
    set_target_properties(${testKernelTargetName} PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(${testKernelTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testKernelTargetName}D")
    set_target_properties(${testKernelTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
else(MSVC)
    set_target_properties(${testKernelTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testKernelTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

if(WIN32)
    INSTALL(TARGETS ${testKernelTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testKernelTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testKernelTargetName}
    )
else(WIN32)
    INSTALL(TARGETS ${testKernelTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testKernelTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testKernelTargetName}
    )
endif(WIN32)