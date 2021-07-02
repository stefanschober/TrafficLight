# BSP/Arm/Drivers/STM32F0xx_HAL_Driver/Src
set(_src "")
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND _src stm32f0xx_hal.c)
foreach(file IN ITEMS cortex dma flash flash_ex gpio i2c i2c_ex pwr pwr_ex rcc rcc_ex tim tim_ex uart uart_ex)
    list(APPEND _src stm32f0xx_hal_${file}.c)
endforeach()

target_sources(${TGT} PRIVATE ${_src})
target_include_directories(${TGT} PRIVATE ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
