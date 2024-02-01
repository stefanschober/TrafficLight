# BSP/Windows
set(_src "")
list(APPEND _src bsp.c)
target_sources(${TGT} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
