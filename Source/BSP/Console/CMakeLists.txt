# BSP/Windows
if(CONFIG_UNIT_TEST)
    target_sources(${TGT} PRIVATE
        bsp-qutest.c
    )
else()
    target_sources(${TGT} PRIVATE
        bsp.c
    )
endif()

# compiler options
target_compile_options(${TGT} PUBLIC
    -pthread
    $<$<BOOL:${WIN32}>:-mconsole>
)

# linker options
target_link_options(${TGT} PUBLIC
    -pthread
    $<$<BOOL:${WIN32}>:-mconsole>
    LINKER:-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
)
