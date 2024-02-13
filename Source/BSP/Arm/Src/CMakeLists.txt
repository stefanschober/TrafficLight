# BSP/Arm/Src
target_sources(${TGT} PRIVATE 
    main.c
    stm32f0xx_hal_msp.c
    stm32f0xx_it.c
    system_stm32f0xx.c
)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
