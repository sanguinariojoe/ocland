# ===================================================== #
# Install                                               #
# ===================================================== #

SET(CMAKE_INSTALL_INCLUDEDIR include/ocland)

IF(OCLAND_SERVER OR OCLAND_CLIENT)
INSTALL(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/common
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)
ENDIF(OCLAND_SERVER OR OCLAND_CLIENT)
IF(OCLAND_SERVER)
INSTALL(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/client
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)
ENDIF(OCLAND_SERVER)
IF(OCLAND_CLIENT)
INSTALL(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/server
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)
ENDIF(OCLAND_CLIENT)

