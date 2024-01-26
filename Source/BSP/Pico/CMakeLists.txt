# BSP/Pico

target_sources(${TGT} PRIVATE bsp.c)
listModules(${CMAKE_CURRENT_SOURCE_DIR} bsp.c)

target_compile_definitions(${TGT}
    PUBLIC
        $<$<BOOL:${CONFIG_PICO_CMSIS}>:PICO_CMSIS_RENAME_EXCEPTIONS=1>
#        $<$<BOOL:${CONFIG_PICO_CMSIS}>:PICO_NO_RAM_VECTOR_TABLE=1>
)
target_link_libraries(${TGT}
    PUBLIC
        pico_stdlib
        cmsis_core
        hardware_exception
        hardware_irq
        $<$<CONFIG:Spy>:hardware_uart>
)