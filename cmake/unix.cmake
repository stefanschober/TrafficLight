#default port is posix
set(PORT posix)

# linker options
target_link_options(${TGT} PUBLIC
    -pthread
    $<$<BOOL:${WIN32}>:-mconsole>
    $<IF:$<CONFIG:Debug,Spy>,-g3,-g0>
    LINKER:--cref,--gc-sections,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
)
