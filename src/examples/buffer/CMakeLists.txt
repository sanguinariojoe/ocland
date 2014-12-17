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
# App target                                         #
# ===================================================== #
SOURCE_GROUP("test_buffer" FILES ${TEST_BUFFER_CPP_SRCS})

SET(testBufferTargetName test_buffer)

add_executable(${testBufferTargetName} ${TEST_BUFFER_CPP_SRCS})

target_link_libraries(${testBufferTargetName} ${DEP_LIBS})

# ===================================================== #
# Install App                                           #
# ===================================================== #
if(MSVC)
    set_target_properties(${testBufferTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testBufferTargetName}D")
    set_target_properties(${testBufferTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # dirty hack to avoid Debug/Release subdirectory
    set_target_properties(${testBufferTargetName} PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(${testBufferTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testBufferTargetName}D")
    set_target_properties(${testBufferTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
else(MSVC)
    set_target_properties(${testBufferTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testBufferTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

if(WIN32)
    INSTALL(TARGETS ${testBufferTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testBufferTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testBufferTargetName}
    )
else(WIN32)
    INSTALL(TARGETS ${testBufferTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testBufferTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testBufferTargetName}
    )
endif(WIN32)