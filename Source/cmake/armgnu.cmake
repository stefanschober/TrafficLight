execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/BSP/Arm/${TGT}-qk.ld ${CMAKE_BINARY_DIR}/${SCATTER_FILE}
    # COMMAND ${CMAKE_C_COMPILER} -E -P ${SCATTER_FILE}.cpp -o ${SCATTER_FILE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE _ld_ok
)

if(NOT CONFIG_PICO)
    # set global C compiler flags
    # set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-mcpu=cortex-m0 -mthumb --specs=nano.specs --specs=nosys.specs")

    # set default compiler options, valid for all USC2 projects
    target_compile_definitions(${TGT}
        PUBLIC
            V_COMP_KEIL
            __TARGET_CPU_CORTEX_M0
    )

    target_compile_options(${QPLIB}
        PUBLIC
            -Os
            -g3
            -fmessage-length=0
            -fdata-sections
            -ffunction-sections)

    target_compile_options(${TGT}
        PUBLIC
            -Os
            -g3
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
            -g3
            -Wl,--cref,--gc-sections,-Map=$<TARGET_NAME:${TGT}>.map
    )
endif()