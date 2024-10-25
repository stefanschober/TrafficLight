# BSP/Windows
target_sources(${TGT} PRIVATE
    bsp.c
    bsp-gui.c
    trafficlight-gui.rc
)

# additional compile definitions
target_compile_definitions(${TGT} PUBLIC QWIN_GUI)

# compiler options
target_compile_options(${TGT} PUBLIC
    -pthread
    -mwindows
    -mwin32
)

# linker options
target_link_options(${TGT} PUBLIC
    -pthread
    -mwindows
    LINKER:-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
)
