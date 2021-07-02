# cmake file to create the complete flashable application
# the following information is required via -D options to the cmake call
# PARAM_IN - the complete path to the parameter eeprom data file (binary)
# PROFILE_IN - the complete path to the profile eeprom data file (binary)
# HEXFILE - the flashable application in Intel HEX format as created by
# objcopy/fromelf
if(NOT (PARAM_IN AND PROFILE_IN AND HEXFILE))
    message(FATAL_ERROR "usage ${CMAKE_CURRENT_LIST_FILE} -DHEXFILE=<app hex file> -DPARAM_IN=<param data> -DPROFILE_IN=<profile data> -[DARMCC=ON]")
endif()

# find the required helper programs
find_program(SREC_CAT NAMES srec_cat srec_cat.exe)
if(NOT SREC_CAT)
	message(FATAL_ERROR "Ooops - the required program 'srec_cat' cannot be found!")
endif()

find_program(AWK NAMES awk awk.exe)
if(NOT AWK)
	message(FATAL_ERROR "Ooops - the required program 'awk' cannot be found!")
endif()

# set the AWK command line
if(ARMCC)
    set(AWKCMDpar "/eepromParameter[ ]*0x[0-9a-fA-F]+/ { print \$2 }")
    set(AWKCMDprf "/eepromVariant[ ]*0x[0-9a-fA-F]+/ { print \$2 }")
else()
    set(AWKCMDpar "/0x[0-9a-fA-F]+[ ]+eepromParameter/ { print $1 }")
    set(AWKCMDprf "/0x[0-9a-fA-F]+[ ]+eepromVariant/ { print $1 }")
endif()

# extract the eeprom memory address from the map file
execute_process(
    COMMAND ${AWK} "${AWKCMDpar}" ${CMAKE_BINARY_DIR}/${HEXFILE}.map
    RESULT_VARIABLE NOK
    OUTPUT_VARIABLE EEPAR_ADDR
)
if(NOK OR (NOT EEPAR_ADDR))
    message(FATAL_ERROR "could not determine the eepromParameter structure's memory address")
endif()

execute_process(
    COMMAND ${AWK} "${AWKCMDprf}" ${CMAKE_BINARY_DIR}/${HEXFILE}.map
    RESULT_VARIABLE NOK
    OUTPUT_VARIABLE EEPRF_ADDR
)
if(NOK OR (NOT EEPRF_ADDR))
    message(FATAL_ERROR "could not determine the eepromVariant structure's memory address")
endif()

# create eepromData.hex and *.cpl.hex files
set(SRECCMD_FILE ${CMAKE_BINARY_DIR}/_srec.par)
set(EEPROM_DATA_FILE eepromData.hex)
# message(STATUS "${SREC_CAT} ${PARAM_IN} -bin -offset ${EEPAR_ADDR} ${PROFILE_IN} -bin -offset ${EEPRF_ADDR} -o ${EEPROM_DATA_FILE} -intel")
file(WRITE  ${SRECCMD_FILE} "${PARAM_IN} -bin -offset ${EEPAR_ADDR}\n")
file(APPEND ${SRECCMD_FILE} "${PROFILE_IN} -bin -offset ${EEPRF_ADDR}\n")
file(APPEND ${SRECCMD_FILE} "-o ${EEPROM_DATA_FILE} -intel\n")
execute_process(
    COMMAND ${SREC_CAT} "@${SRECCMD_FILE}"
    RESULT_VARIABLE NOK
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE  STDERR
)
if(NOK)
    message (FATAL_ERROR "creating ${EEPROM_DATA_FILE} failed with message ${STDOUT} ${STDERR}")
endif()

file(WRITE  ${SRECCMD_FILE} "${HEXFILE}.hex -intel -exclude -over ${EEPROM_DATA_FILE} -intel\n")
file(APPEND ${SRECCMD_FILE} "${EEPROM_DATA_FILE} -intel -o __tmp.s19\n")
execute_process(
    COMMAND ${SREC_CAT} "@${SRECCMD_FILE}"
    RESULT_VARIABLE NOK
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE  STDERR
)
if(NOK)
    message (FATAL_ERROR "creating ${HEXFILE}.cpl.hex failed with message ${STDOUT} ${STDERR}")
endif()

file(WRITE  ${SRECCMD_FILE} "__tmp.s19 -fill 0x00 -over __tmp.s19 -o ${HEXFILE}.cpl.hex -intel\n")
execute_process(
    COMMAND ${SREC_CAT} "@${SRECCMD_FILE}"
    RESULT_VARIABLE NOK
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE  STDERR
)
if(NOK)
    message (FATAL_ERROR "creating ${HEXFILE}.cpl.hex failed with message ${STDOUT} ${STDERR}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND}  -E remove -f __tmp.s19 ${SRECCMD_FILE}
    RESULT_VARIABLE NOK
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE  STDERR
)
if(NOK)
    message (FATAL_ERROR "Command (${CMAKE_COMMAND}  -E remove -f __tmp.s19 ${SRECCMD_FILE}) failed with message ${STDOUT} ${STDERR}")
endif()

