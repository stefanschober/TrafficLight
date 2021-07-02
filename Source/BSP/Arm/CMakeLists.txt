# BSP/TLE
foreach(dir IN ITEMS Src TrafficLight/SW4STM32 Drivers/STM32F0xx_HAL_Driver/Src)
    add_subdirectory(${dir})
endforeach()

set(_src "")
set(_inc "")
list(APPEND _src bsp.c)
list(APPEND _inc . Inc Drivers/CMSIS/Device/ST/STM32F0xx/Include Drivers/CMSIS/Include Drivers/STM32F0xx_HAL_Driver/Inc)

target_sources(${TGT} PRIVATE ${_src})
target_include_directories(${TGT} PRIVATE ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

