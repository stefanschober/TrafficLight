execute_process(
	COMMAND ${CMAKE_C_COMPILER} -E -P ${SCATTER_FILE}.cpp -o ${SCATTER_FILE}
	COMMAND ${CMAKE_C_COMPILER} -E -P ${SCATTER_TPL}_sect.cpp -o ${SCATTER_TPL}_sect.ld
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	RESULT_VARIABLE _ld_ok
)

# set default compiler options, valid for all USC2 projects
target_compile_definitions(${TGT}
	PUBLIC
		V_COMP_KEIL
		__TARGET_CPU_CORTEX_M0
)

add_compile_options(
	-mcpu=cortex-m0
)

# set default linker options, valid for all USC2 projects
target_link_options(${TGT}
	PUBLIC
        -T ${SCATTER_FILE}
		-nostartfiles
		-static
)
