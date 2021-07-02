# QM/Source
set(_src "")
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND _src TLmain.c TLblinker.c TLtraffic.c TLpedestrian.c)
target_sources(${TGT} PRIVATE ${_src})
target_include_directories(${TGT} PRIVATE ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

