# BSP/Raspi
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${TGT} PRIVATE bsp.c)

if(CONFIG_GUI)
    target_sources(${TGT} PRIVATE bsp-gui.c)
    include(gtk)
endif()