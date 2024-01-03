#default port is posix
set(PORT posix)

# additional compile definitions
target_compile_definitions(${TGT}
    PUBLIC
        USC2_POSIX
        $<$<BOOL:${CONFIG_RASPI_IO}>:RASPI>
        $<$<NOT:$<BOOL:${CONFIG_PIGPIO}>>:PIGPIO_LIB>
)

# compiler options
add_compile_options(
    $<IF:$<CONFIG:Debug,Spy>,-g3,-g0>
    $<IF:$<CONFIG:Debug,Spy>,-O0,-Os>
    -Wall
    -fmessage-length=0
    -ffunction-sections
    -fdata-sections
    -pthread
#   $<$<BOOL:${CONFIG_RASPI}>:-v>
)

# linker options
target_link_options(${TGT}
    PUBLIC
        -pthread
        $<IF:$<CONFIG:Debug,Spy>,-g3,-g0>
        -Wl,$<$<C_COMPILER_ID:GNU>:--cref,>--gc-sections,-Map=$<TARGET_NAME:${TGT}>.map
#        $<$<BOOL:${CONFIG_RASPI}>:-v>
)

target_link_libraries(${TGT} PRIVATE $<$<BOOL:${CONFIG_RASPI_IO}>:$<IF:$<BOOL:${CONFIG_PIGPIO}>,pigpiod_if2,pigpio>>)

# add GTK+-3.0 support
if(CONFIG_GUI)
    if(CONFIG_RASPI)
        set(ENV{PKG_CONFIG_PATH} "/usr/lib/arm-linux-gnueabihf/pkgconfig/")
    endif()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 gtk+-3.0) # QUIET IMPORTED_TARGET)

    if (NOT GTK3_FOUND)
        message(FATAL_ERROR "The package GTK3, required for this build, could not be found on the system!")
    endif()

    # Add other flags to the compiler
    target_compile_options(${TGT} PRIVATE ${GTK3_CFLAGS_OTHER})

    # Setup CMake to use GTK+, tell the compiler where to look for headers
    # and to the linker where to look for libraries
    target_include_directories(${TGT} PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_directories(${TGT} PRIVATE ${GTK3_LIBRARY_DIRS})
    target_link_options(${TGT} PRIVATE ${GTK3_LDFLAGS_OTHER})

    # Link the target to the GTK+ libraries
    target_link_libraries(${TGT} PRIVATE ${GTK3_LIBRARIES}) # PkgConfig::GTK3)
endif()

include(custom_commands)
