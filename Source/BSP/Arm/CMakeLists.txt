# BSP/TLE
if(NOT CONFIG_PICO)
    foreach(dir IN ITEMS Src TrafficLight/SW4STM32 Drivers/STM32F0xx_HAL_Driver/Src)
        add_subdirectory(${dir})
    endforeach()
    target_include_directories(${TGT} PRIVATE
        Drivers/CMSIS/Device/ST/STM32F0xx/Include Drivers/CMSIS/Include
        Drivers/CMSIS/Include
        Drivers/STM32F0xx_HAL_Driver/Inc
        Inc
        Res
    )
endif()

target_sources(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/$<IF:$<BOOL:${CONFIG_PICO}>,bsp-pico.c,bsp.c>)
target_include_directories(${TGT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
