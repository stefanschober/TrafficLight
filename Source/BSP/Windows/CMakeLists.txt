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
