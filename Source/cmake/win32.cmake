# the WIN32 target is a GUI-only target system
if(NOT CONFIG_UNIT_TEST)
    set(CONFIG_GUI ON)
endif()

#default port is WIN32
set(PORT win32)
set(MCU Host-CPU)

# additional compile definitions
target_compile_definitions(${TGT}
    PUBLIC
        USC2_WIN32
)

# compiler options
target_compile_options(${TGT}
    PUBLIC
        $<IF:$<C_COMPILER_ID:GNU>,$<IF:$<BOOL:${CONFIG_GUI}>,-mwindows,-mconsole>, >
        -Os
        -g3
        -Wall
        -fmessage-length=0
        -ffunction-sections
        -fdata-sections
)

# linker options
target_link_options(${TGT}
    PUBLIC
        $<IF:$<BOOL:${CONFIG_GUI}>,-mwindows,-mconsole>
        -Wl,--cref,--gc-sections,-Map=$<TARGET_NAME:${TGT}>.map
)

# add windows socket library for Q_SPY/Q_UTEST configurations
if(CONFIG_QSPY)
    target_link_libraries(${TGT} PRIVATE ws2_32)
endif()

# include(custom_commands)
