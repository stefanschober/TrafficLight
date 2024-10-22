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
        LINKER:--cref,--gc-sections,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
#        $<$<BOOL:${CONFIG_RASPI}>:-v>
)

target_link_libraries(${TGT} PRIVATE $<$<BOOL:${CONFIG_RASPI_IO}>:$<IF:$<BOOL:${CONFIG_PIGPIO}>,pigpiod_if2,pigpio>>)

include(custom_commands)
