# BSP/Raspi
target_sources(${TGT} PRIVATE
    bsp.c
    $<$<BOOL:${CONFIG_GUI}>:${CMAKE_CURRENT_SOURCE_DIR}/bsp-gui.c>
    $<$<BOOL:${CONFIG_GUI}>:${CMAKE_CURRENT_SOURCE_DIR}/resources.c>
)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
