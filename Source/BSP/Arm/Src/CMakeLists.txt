# BSP/Arm/Src
set(_src "")
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND _src main.c stm32f0xx_hal_msp.c stm32f0xx_it.c system_stm32f0xx.c)
target_sources(${TGT} PRIVATE ${_src})
target_include_directories(${TGT} PRIVATE ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
