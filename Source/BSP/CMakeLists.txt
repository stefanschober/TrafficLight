# CMake the qpc libraries for different targets
# target may be "Windows", "Posix", "TLE984x"
# BSP/Windows
if(PORT MATCHES arm)
    if(CONFIG_PICO)
        message("BSP Pico")
        add_subdirectory(Pico)
    else()
        message("BSP Arm")
        add_subdirectory(Arm)
    endif()
else() # must be posix or win32
    if(CONFIG_GUI)
        if(PORT MATCHES win32)
            message("BSP win32")
            enable_language(RC)
            add_subdirectory(Windows)
        else()
            message("BSP Raspi")
            add_subdirectory(Raspi)
        endif()
    else()
        message("BSP Console")
        add_subdirectory(Console)
    endif()
endif()

target_sources(${TGT} PRIVATE bsp-common.c)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
