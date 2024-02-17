# the WIN32 target is a GUI-only target system
if(CONFIG_UNIT_TEST)
    set(CONFIG_GUI OFF)
elseif(NOT DEFINED CONFIG_GUI)
    set(CONFIG_GUI ON)
endif()
set(QPC_CFG_GUI ${CONFIG_GUI})

if(NOT WIN32)
    set(WIN32 ON)
endif()

#default port is WIN32
set(PORT win32)

if(CONFIG_GUI AND CONFIG_GTK)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 gtk+-3.0) # QUIET IMPORTED_TARGET)

    if (NOT GTK3_FOUND)
        message(FATAL_ERROR "The package GTK3, required for this build, could not be found on the system!")
    endif()

    # enable GTK support for sources
    target_compile_definitions(${TGT} PRIVATE USE_GTK=1)
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

# additional compile definitions
target_compile_definitions(${TGT}
    PUBLIC
        USC2_WIN32
        $<$<CONFIG:Spy>:Q_SPY>
        $<$<BOOL:${CONFIG_GUI}>:QWIN_GUI>
)

# compiler options
add_compile_options(
    -pthread
)

target_compile_options(${TGT}
    PUBLIC
        $<IF:$<C_COMPILER_ID:GNU>,$<IF:$<BOOL:${CONFIG_GUI}>,-mwindows,-mconsole>, >
)

# linker options
target_link_options(${TGT}
    PUBLIC
        $<IF:$<BOOL:${CONFIG_GUI}>,-mwindows,-mconsole>
        LINKER:-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
)

include(custom_commands)
