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
# App target                                         #
# ===================================================== #
SOURCE_GROUP("test_program" FILES ${TEST_PROGRAM_CPP_SRCS})

SET(testProgramTargetName test_program)

add_executable(${testProgramTargetName} ${TEST_PROGRAM_CPP_SRCS})

target_link_libraries(${testProgramTargetName} ${DEP_LIBS})

# ===================================================== #
# Install App                                           #
# ===================================================== #
if(MSVC)
    set_target_properties(${testProgramTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testProgramTargetName}D")
    set_target_properties(${testProgramTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # dirty hack to avoid Debug/Release subdirectory
    set_target_properties(${testProgramTargetName} PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(${testProgramTargetName} PROPERTIES DEBUG_OUTPUT_NAME "${testProgramTargetName}D")
    set_target_properties(${testProgramTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
else(MSVC)
    set_target_properties(${testProgramTargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    set_target_properties(${testProgramTargetName} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

if(WIN32)
    INSTALL(TARGETS ${testProgramTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testProgramTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testProgramTargetName}
    )
else(WIN32)
    INSTALL(TARGETS ${testProgramTargetName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${testProgramTargetName}
        LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/${testProgramTargetName}
    )
endif(WIN32)