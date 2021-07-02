# BSP/Arm/TrafficLight/SW4ST32
set(_src "")
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND _src startup_stm32f091xc.S syscalls.c)
target_sources(${TGT} PRIVATE ${_src})
target_include_directories(${TGT} PRIVATE ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
