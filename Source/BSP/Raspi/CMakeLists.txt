# BSP/Raspi
target_include_directories(${TGT} PRIVATE .)
target_sources(${TGT} PRIVATE bsp.c)

if(CONFIG_GUI)
    target_sources(${TGT} PRIVATE bsp-gui.c)
    include(gtk)
endif()

# additional compile definitions
target_compile_definitions(${TGT} PUBLIC
    USC2_POSIX
    $<$<BOOL:${CONFIG_RASPI_IO}>:RASPI>
    $<$<NOT:$<BOOL:${CONFIG_PIGPIO}>>:PIGPIO_LIB>
)

# compiler options
target_compile_options(${TGT} PUBLIC
    $<IF:$<CONFIG:Debug,Spy>,-g3,-g0>
    $<IF:$<CONFIG:Debug,Spy>,-O0,-Os>
    -Wall
    -fmessage-length=0
    -ffunction-sections
    -fdata-sections
    -pthread
)

if(CONFIG_RASPI_IO)
    target_link_libraries(${TGT} PRIVATE $<IF:$<BOOL:${CONFIG_PIGPIO}>,pigpiod_if2,pigpio>)
endif()
