# set default compiler options, valid for all USC2 projects
target_compile_definitions(${TGT}
    PUBLIC
        $<$<NOT:$<BOOL:${CONFIG_DEBUG}>>:NDEBUG>

)

target_compile_definitions(${QPLIB}
    PUBLIC
        $<$<NOT:$<BOOL:${CONFIG_DEBUG}>>:NDEBUG>
)

target_compile_options(${QPLIB}
    PUBLIC
        $<IF:$<BOOL:${CONFIG_DEBUG}>,-O0 -g3,-Os -g0>
)

target_compile_options(${TGT}
    PUBLIC
        $<IF:$<BOOL:${CONFIG_DEBUG}>,-O0 -g3,-Os -g0>
)

# set default linker options, valid for all USC2 projects
target_link_options(${TGT}
    PUBLIC
        $<IF:$<BOOL:${CONFIG_DEBUG}>,-g3,-g0>
)

if(NOT CONFIG_PICO)
    # prepare linker script
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/BSP/Arm/${SCATTER_TPL}_sect.ld ${CMAKE_BINARY_DIR}/${SCATTER_TPL}_sect.ld
        COMMAND ${CMAKE_C_COMPILER} -E -P ${SCATTER_FILE}.cpp -o ${SCATTER_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE _ld_ok
    )

    # set global C compiler flags
    # set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-mcpu=cortex-m0 -mthumb --specs=nano.specs --specs=nosys.specs")

    # set default compiler options, valid for all USC2 projects
    target_compile_definitions(${TGT}
        PUBLIC
            V_COMP_KEIL
            __TARGET_CPU_CORTEX_M0
            STM32F091xC
    )

    target_compile_options(${QPLIB}
        PUBLIC
            -fmessage-length=0
            -fdata-sections
            -ffunction-sections
    )

    target_compile_options(${TGT}
        PUBLIC
            -fmessage-length=0
            -fdata-sections
            -ffunction-sections 
    )

    # set default linker options, valid for all USC2 projects
    target_link_options(${TGT}
        PUBLIC
            -T ${SCATTER_FILE}
            -nostartfiles
            -static
            -Wl,--cref,--gc-sections,-Map=$<TARGET_NAME:${TGT}>.map
    )
endif()
