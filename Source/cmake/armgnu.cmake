
execute_process(
	COMMAND ${CMAKE_C_COMPILER} -E -I  ${CMAKE_SOURCE_DIR} -P ${SCATTER_FILE}.cpp -o ${SCATTER_FILE}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	RESULT_VARIABLE _ld_ok
)

# set default compiler options, valid for all USC2 projects
target_compile_definitions(${TGT}
	PUBLIC
		V_COMP_KEIL
		__TARGET_CPU_CORTEX_M0
)

if(MCU STREQUAL STM32F091xC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0")
endif()

# set default linker options, valid for all USC2 projects
target_link_options(${TGT}
	PUBLIC
	LINKER:-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_PROJECT_NAME}.map
	$<$<NOT:$<BOOL:${CONFIG_PICO}>>:-T ${SCATTER_FILE}>
	-nostartfiles
	-static
	-mcpu=cortex-$<IF:$<STREQUAL:${MCU},STM32F091xC>,m0plus,m0>
)
