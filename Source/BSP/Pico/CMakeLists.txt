# BSP/Pico

target_sources(${TGT} PRIVATE bsp.c)
listModules(${CMAKE_CURRENT_SOURCE_DIR} bsp.c)

target_link_libraries(${TGT} PUBLIC pico_stdlib pico_time hardware_exception)


