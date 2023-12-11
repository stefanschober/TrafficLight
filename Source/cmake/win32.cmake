# the WIN32 target is a GUI-only target system
if(NOT CONFIG_UNIT_TEST)
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
)

# add windows socket library for Q_SPY/Q_UTEST configurations
if(CONFIG_QSPY)
    target_link_libraries(${TGT} PRIVATE ws2_32)
endif()

include(custom_commands)
