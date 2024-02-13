# BSP/Windows
if(CONFIG_UNIT_TEST)
    target_sources(${TGT} PRIVATE
        bsp-qutest.c
    )
else()
    target_sources(${TGT} PRIVATE
        bsp.c
    )
endif()