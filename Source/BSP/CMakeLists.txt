# CMake the qpc libraries for different targets
# target may be "Windows", "Posix", "TLE984x"
# BSP/Windows
if(WINDOWS OR WIN32)
    enable_language(RC)
    add_subdirectory(Windows)
elseif(UNIX)
    if(CONFIG_CONSOLE)
        add_subdirectory(Linux_Console)
    else()
        add_subdirectory(Raspi)
    endif()
elseif(CONFIG_PICO)
    add_subdirectory(Pico)
else()
    add_subdirectory(Arm)
endif()
target_sources(${TGT} PRIVATE bsp-common.c)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
