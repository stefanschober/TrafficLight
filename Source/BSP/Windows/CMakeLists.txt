# BSP/Windows
set(_src "")
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})
if(USE_GTK)
    list(APPEND _src bsp-gtk.c)
else()
    list(APPEND _src bsp.c bsp-gui.c trafficlight-gui.rc)
endif()
target_sources(${TGT} PRIVATE ${_src})
target_include_directories(${QPLIB} PUBLIC ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

unset(_src)
