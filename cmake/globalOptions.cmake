# set general defines and options
target_compile_definitions(${TGT}
    PRIVATE
        $<UPPER_CASE:${IMAGE}>
        TARGET=${TGT}
        $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
        KERNEL_$<IF:$<STREQUAL:${CONFIG_KERNEL},QK>,QK,QV>=1
        $<$<CONFIG:Spy>:Q_SPY=1>
        $<$<AND:$<CONFIG:Spy>,$<BOOL:${CONFIG_UNIT_TEST}>>:Q_UTEST=1>
        $<$<BOOL:${CONFIG_GUI}>:QWIN_GUI=1>
)

target_compile_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

target_link_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

if((NOT CONFIG_GUI) AND (NOT (PORT MATCHES ARM)))
    target_compile_definitions(qpc PUBLIC QF_CONSOLE)
endif()
