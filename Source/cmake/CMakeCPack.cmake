# CPack configuration for USC2

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Webasto SE")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/Install.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The USC2 standard SW application")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License")
set(CPACK_PACKAGING_INSTALL_PREFIX "./${PROJECT_NAME}")

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_SOURCE_IGNORE_FILES
    "${PROJECT_BINARY_DIR}"
    "/\\\\.git.*"
    "/\\\\..*(project|settings)"
    "/CreatePosition"
    "/_(Listings|Objects)"
    "/Makefile.*"
    "/build.sh"
    "/.*PositionBin.*"
    "\\\\.(mak|xlsx)$"
)
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ;TBZ2")
set(CPACK_GENERATOR "${CPACK_SOURCE_GENERATOR}")

if(UNIX AND (CMAKE_SYSTEM_NAME MATCHES Linux}))
    list(APPEND CPACK_GENERATOR "RPM")
endif()

if(FALSE)
    if(WIN32 OR MINGW)
        list(APPEND CPACK_GENERATOR "NSIS")
        set(CPACK_NSIS_PACKAGE_NAME "${TGT}")
        set(CPACK_NSIS_CONTACT "Stefan Schober")
        set(CPACK_NSIS_NABLE_UNINSTALL_BEFORE_INSTALL ON)
    endif()
endif()

message(STATUS "CPack generators: ${CPACK_GENERATOR}")

include(CPack)
