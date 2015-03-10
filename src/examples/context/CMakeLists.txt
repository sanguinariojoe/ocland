# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_CONTEXT_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_context" FILES ${TEST_CONTEXT_CPP_SRCS})

SET(testContextTargetName test_context)

add_executable(${testContextTargetName} ${TEST_CONTEXT_CPP_SRCS})

target_link_libraries(${testContextTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testContextTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testContextTargetName})
SET_TESTS_PROPERTIES(${testContextTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
if(MSVC)
    set_target_properties(${testContextTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testContextTargetName}D")
    set_target_properties(${testContextTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # dirty hack to avoid Debug/Release subdirectory
    set_target_properties(${testContextTargetName} PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(${testContextTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testContextTargetName}D")
    set_target_properties(${testContextTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
else(MSVC)
    set_target_properties(${testContextTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testContextTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

if(WIN32)
    INSTALL(TARGETS ${testContextTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testContextTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testContextTargetName}
    )
else(WIN32)
    INSTALL(TARGETS ${testContextTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testContextTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testContextTargetName}
    )
endif(WIN32)