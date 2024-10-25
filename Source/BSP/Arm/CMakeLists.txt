# BSP/TLE

target_include_directories(${TGT} PRIVATE .)

target_compile_definitions(${TGT} PUBLIC
    ${MCU}
    FLASHSIZE=${MCU_FLASH_SIZE}
    STACKSIZE=${STACKSIZE}
    $<$<BOOL:${HEAPSIZE}>:HEAPSIZE=${HEAPSIZE}>
    $<$<NOT:$<BOOL:${CONFIG_LIBINIT}>>:NO_LIBINIT>
)

if(CONFIG_PICO)
    target_compile_definitions(${TGT} PUBLIC
        $<$<BOOL:${CONFIG_PICO_CMSIS}>:PICO_CMSIS_RENAME_EXCEPTIONS=1>
        # $<$<BOOL:${CONFIG_PICO_CMSIS}>:PICO_NO_RAM_VECTOR_TABLE=1>
    )
    target_link_libraries(${TGT} PUBLIC
        pico_stdlib
        cmsis_core
        hardware_exception
        hardware_irq
        hardware_clocks
        $<$<BOOL:${CONFIG_PICO}>:hardware_uart>
    )
    target_sources(${TGT} PRIVATE bsp-pico.c)

    pico_enable_stdio_usb(${TGT} TRUE)
    pico_enable_stdio_uart(${TGT} TRUE)
else()
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

    target_link_options(${TGT} PUBLIC
        LINKER:-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
        -T ${CMAKE_BINARY_DIR}/${SCATTER_FILE}
        -nostartfiles
        -static
        -mcpu=cortex-$<IF:$<STREQUAL:${MCU},STM32F091xC>,m0plus,m0>
    )
    target_sources(${TGT} PRIVATE bsp.c)
endif()
