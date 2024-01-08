# the WIN32 target is a GUI-only target system
if(CONFIG_UNIT_TEST)
    set(CONFIG_GUI OFF)
else()
    set(CONFIG_GUI ON)
endif()
set(QPC_CFG_GUI ${CONFIG_GUI})

if(NOT WIN32)
    set(WIN32 ON)
endif()

#default port is WIN32
set(PORT win32)

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
