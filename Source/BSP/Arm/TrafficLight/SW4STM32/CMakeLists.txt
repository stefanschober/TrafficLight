# BSP/Arm/TrafficLight/SW4ST32
target_sources(${TGT} PRIVATE
    startup_stm32f091xc.c
    syscalls.c
)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
