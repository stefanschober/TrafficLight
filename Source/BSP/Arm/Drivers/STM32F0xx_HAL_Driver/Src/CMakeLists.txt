# BSP/Arm/Drivers/STM32F0xx_HAL_Driver/Src
file(GLOB C_SOURCES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
target_sources(${TGT} PRIVATE
    ${C_SOURCES}
)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
unset(C_SOURCES)
