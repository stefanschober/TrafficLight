# BSP/Windows
set(_src bsp.c bsp-gui.c trafficlight-gui.rc)

target_sources(${TGT} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
unset(_inc)