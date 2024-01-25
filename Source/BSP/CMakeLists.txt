# CMake the qpc libraries for different targets
# target may be "Windows", "Posix", "TLE984x"
# BSP/Windows
if((CONFIG_GUI AND CONFIG_GTK) OR (PORT MATCHES posix))
    if(CONFIG_GUI)
        add_subdirectory(Raspi)
    else()
        add_subdirectory(Linux_Console)
    endif()
elseif(PORT MATCHES win32)
    enable_language(RC)
    add_subdirectory(Windows)
elseif(CONFIG_PICO)
    add_subdirectory(Pico)
else()
    add_subdirectory(Arm)
endif()
target_sources(${TGT} PRIVATE bsp-common.c)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
