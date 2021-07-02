# BSP/Windows
set(_src "")
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND _src bsp.c)
target_sources(${TGT} PRIVATE ${_src})
target_include_directories(qpclib PUBLIC ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)

