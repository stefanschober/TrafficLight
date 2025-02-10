# the WIN32 target is a GUI-only target system
if(CONFIG_UNIT_TEST OR CONFIG_QF_CON)
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

# linker options
target_link_options(${TGT} PUBLIC
    -pthread
    $<IF:$<BOOL:${CONFIG_GUI}>,-mwindows,-mconsole>
    $<IF:$<CONFIG:Debug,Spy>,-g3,-g0>
    LINKER:$<$<C_COMPILER_ID:GNU>:--cref,>--gc-sections,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
)
