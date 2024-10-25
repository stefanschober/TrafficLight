# the WIN32 target is a GUI-only target system
if(CONFIG_UNIT_TEST OR CONFIG_QF_CON)
    set(CONFIG_GUI OFF)
else()
    set(CONFIG_GUI ON)
endif()
set(QPC_CFG_GUI ${CONFIG_GUI})

if(NOT WIN32)
    set(WIN32 ON)
endif()

#default port is WIN32
set(PORT win32)

include(custom_commands)
