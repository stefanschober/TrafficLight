# CMake the qpc libraries for different targets
# target may be "Windows", "Posix", "TLE984x"
# BSP/Windows
if(PORT MATCHES arm)
    message("BSP Arm")
    add_subdirectory(Arm)
elseif(CONFIG_GUI)
    if((PORT MATCHES posix) OR CONFIG_GTK)
        message("BSP GTK")
        add_subdirectory(Raspi)
    else() # must be win32
        message("BSP Win32")
        enable_language(RC)
        add_subdirectory(Windows)
    endif()
else()
    message("BSP Console")
    add_subdirectory(Console)
endif()

target_sources(${TGT} PRIVATE bsp-common.c)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
