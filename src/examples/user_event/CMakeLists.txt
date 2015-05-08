# ===================================================== #
# Link
# ===================================================== #
SET(DEP_LIBS
    ${OPENCL_LIBRARIES}
)

# ===================================================== #
# Sources to compile app                                #
# ===================================================== #
SET(TEST_USER_EVENT_CPP_SRCS
    main.c
)

# ===================================================== #
# App target                                            #
# ===================================================== #
SOURCE_GROUP("test_user_event" FILES ${TEST_USER_EVENT_CPP_SRCS})

SET(testUserEventTargetName test_user_event)

add_executable(${testUserEventTargetName} ${TEST_USER_EVENT_CPP_SRCS})

target_link_libraries(${testUserEventTargetName} ${DEP_LIBS})

# ===================================================== #
# Add app as a test                                     #
# ===================================================== #
ADD_TEST(NAME ${testUserEventTargetName}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${testUserEventTargetName})
SET_TESTS_PROPERTIES(${testUserEventTargetName} PROPERTIES TIMEOUT 180)

# ===================================================== #
# Install App                                           #
# ===================================================== #
if(MSVC)
    set_target_properties(${testUserEventTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testUserEventTargetName}D")
    set_target_properties(${testUserEventTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # dirty hack to avoid Debug/Release subdirectory
    set_target_properties(${testUserEventTargetName} PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(${testUserEventTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testUserEventTargetName}D")
    set_target_properties(${testUserEventTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
else(MSVC)
    set_target_properties(${testUserEventTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testUserEventTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

if(WIN32)
    INSTALL(TARGETS ${testUserEventTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testUserEventTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testUserEventTargetName}
    )
else(WIN32)
    INSTALL(TARGETS ${testUserEventTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testUserEventTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testUserEventTargetName}
    )
endif(WIN32)