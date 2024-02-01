# set general defines and options
target_compile_definitions(${TGT}
    PRIVATE
        $<UPPER_CASE:${IMAGE}>
        TARGET=${TGT}
        $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
        KERNEL_$<IF:$<STREQUAL:${CONFIG_KERNEL},QK>,QK,QV>=1
        $<$<CONFIG:Spy>:Q_SPY>
        $<$<BOOL:${CONFIG_UNIT_TEST}>:Q_UTEST>
        $<$<BOOL:${CONFIG_GUI}>:QWIN_GUI>
)

target_compile_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

target_link_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)
