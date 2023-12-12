# BSP/Windows
set(_src bsp.c trafficlight-gui.rc)

if(CONFIG_GTK)
    list(APPEND _src bsp-gtk.c resources.c)
else()
    list(APPEND _src bsp-gui.c)
endif()

target_sources(${TGT} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
unset(_inc)