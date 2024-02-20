# set general defines and options
target_compile_definitions(${TGT}
    PRIVATE
        $<UPPER_CASE:${IMAGE}>
        TARGET=${TGT}
        $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
        KERNEL_$<IF:$<STREQUAL:${CONFIG_KERNEL},QK>,QK,QV>=1
        $<$<AND:$<CONFIG:Spy>,$<BOOL:${CONFIG_UNIT_TEST}>>:Q_UTEST=1>
        $<$<BOOL:${CONFIG_GUI}>:QWIN_GUI=1>
        $<$<BOOL:${CONFIG_QF_CON}>:QF_CONSOLE=1>
        $<$<BOOL:${CONFIG_RASPI_IO}>:RASPI_IO=1>
        $<$<AND:$<BOOL:${CONFIG_RASPI_IO}>,$<BOOL:${CONFIG_PIGPIO_LIB}>>:PIGPIO_LIB=1>
)

target_compile_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

target_link_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

if(CONFIG_QF_CON)
    target_compile_definitions(qpc PRIVATE QF_CONSOLE=1)
endif()
