# some later used functions
function(findParFile)
    set(options)
    set(oneValueArgs RESULT FILENAME)
    set(multiValueArgs)
    cmake_parse_arguments(FPF "${options}" "${oneValueArgs}"  "${multiValueArgs}" ${ARGN})
    if (NOT (FPF_RESULT AND FPF_FILENAME))
        message(FATAL_ERROR "usage: findParFile(RESULT <resultVar> FILENAME <parfile name>)")
    endif()

    set(_fnx ${FPF_FILENAME})
    string(TOUPPER ${_fnx} _fnu)
    string(TOLOWER ${_fnx} _fnl)

    find_file(${FPF_RESULT}
        NAMES
            ${_fnx} ${_fnx}.bin
            ${_fnu} ${_fnu}.BIN
            ${_fnl} ${_fnl}.bin
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_PROJECT_SOURCE_DIR}
        NO_DEFAULT_PATH
        NO_PACKAGE_ROOT_PATH
        NO_CMAKE_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
endfunction()

function(copyParFile)
    set(options)
    set(oneValueArgs RESULT IN TEMPLATE)
    set(multiValueArgs)

    cmake_parse_arguments(CPF "${options}" "${oneValueArgs}"  "${multiValueArgs}" ${ARGN})
    if (NOT (CPF_RESULT AND CPF_IN AND CPF_TEMPLATE))
        message(FATAL_ERROR "usage: copyParFile(RESULT <resultVar> IN <inFile> TEMPLATE <fileName template>)")
    endif()

    set(_out ${CMAKE_CURRENT_BINARY_DIR}/${CPF_TEMPLATE}.bin)
    if(NOT EXISTS ${_out})
        file(GENERATE OUTPUT ${_out} INPUT ${CPF_IN})
    endif()
    
    set(${CPF_RESULT} ${_out} PARENT_SCOPE)
endfunction()

function(getTargetName _name _tgt)
    get_target_property(_n ${_tgt} OUTPUT_NAME)
    get_target_property(_p ${_tgt} PREFIX)
    
    if(NOT _p)
        set(_p "")
    endif()
    if(NOT _n)
        get_target_property(_n ${_tgt} NAME)
        if(NOT _n)
            set(_n ${CMAKE_PROJECT_NAME})
        endif()
    endif()
    
    set(${_name} "${_p}${_n}" PARENT_SCOPE)
endfunction()

#default port is arm-cm
set(PORT arm-cm)

if(CONFIG_PICO)
    message(STATUS "initializing Raspberry Pi Pico SDK")
    pico_sdk_init()

    pico_enable_stdio_usb(${TGT} TRUE)
    pico_enable_stdio_uart(${TGT} TRUE)
    pico_add_extra_outputs(${TGT})

    set(MCU RP2040)
    set(INCFILE armgnu)
else()
    # MCU based settings (size of flash memory)
    if(NOT MCU)
        message(WARNING "No MCU specified. Setting MCU to STM32F091xC!")
        set(MCU STM32F091xC)
    endif()

    # no GUI on embedded target
    if(CONFIG_GUI)
        message(WARNING "GUI is not supported on embedded targets. Resetting CONFIG_GUI!")
	    set(CONFIG_GUI OFF)
    endif()

    if(NOT CONFIG_LIBINIT)
        set(NO_LIBINIT true)
    endif()

    if(MCU STREQUAL "TLE9842QX")
        set(MCU_FLASH_SIZE 36)
    elseif(MCU STREQUAL "TLE9842_2QX")
        set(MCU_FLASH_SIZE 40)
    elseif(MCU STREQUAL "TLE984[35]QX")
        set(MCU_FLASH_SIZE 48)
    elseif(MCU STREQUAL "TLE9843_2QX")
        set(MCU_FLASH_SIZE 52)
    elseif(MCU MATCHES "TLE9844[^ \t]*QX")
        set(MCU_FLASH_SIZE 64)
    elseif(MCU MATCHES "STM32F091[^ \t]*C")
        set(MCU_FLASH_SIZE 256)
    else()
            message(FATAL_ERROR "Unknown MCU ${MCU} specified!")
    endif()

    # Defaults for stack and heap memory
    if(NOT STACKSIZE)
        set(STACKSIZE 0x0200)
    endif()
    if(NOT HEAPSIZE)
        set(HEAPSIZE 0)
    endif()

    # create a .elf output file
    set_target_properties(${TGT}
	    PROPERTIES
		    SUFFIX ".elf"
    )

    # prepare the toolchain specific behavior

    # the linker script will be constructed from
    # the file ${SCATTER_FILE}.in located in the top level source
    # directory (together with this CMakeLists.txt)
    if(NOT DEFINED SCATTER_TPL)
        set(SCATTER_TPL "stm32f091")
    endif()
    set(MKHEX ${FROMELF})
    set(MKHEX_ARGS --i32combined --output=$<TARGET_NAME:${TGT}>.hex $<TARGET_FILE_NAME:${TGT}>)
    set(MKSIZE ${FROMELF})
    set(MKSIZE_ARGS --text -z $<TARGET_FILE_NAME:${TGT}>)

    if(CMAKE_C_COMPILER_ID STREQUAL "ARMCC")
        find_program(FROMELF
            NAMES
                fromelf fromelf.exe
            PATHS
                ${CMAKE_FIND_ROOT_PATH}
            PATH_SUFFIXES
                bin
        )
        if(NOT FROMELF)
            message(FATAL_ERROR "Ooops - the KEIL/ARMCC program 'fromelf' cannot be found")
        endif()
        set(MKHEX ${FROMELF})
        set(MKHEX_ARGS --i32combined --output=$<TARGET_NAME:${TGT}>.hex $<TARGET_FILE_NAME:${TGT}>)
        set(MKSIZE ${FROMELF})
        set(MKSIZE_ARGS --text -z $<TARGET_FILE_NAME:${TGT}>)

        set(SCATTER_FILE ${SCATTER_TPL}.sct)
        set(SCATTER_IN ${SCATTER_FILE}.in)
        set(SCATTER_OUT ${SCATTER_FILE})
        set(INCFILE armcc)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        if(NOT CMAKE_OBJCOPY)
            message(FATAL_ERROR "Ooops - the GNU/binutils program 'objcopy' cannot be found!")
        endif()
        find_program(CMAKE_SIZE NAMES ${_CMAKE_TOOLCHAIN_PREFIX}size${_CMAKE_TOOLCHAIN_SUFFIX} HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
        set(MKHEX ${CMAKE_OBJCOPY})
        set(MKHEX_ARGS -O ihex $<TARGET_FILE_NAME:${TGT}> $<TARGET_NAME:${TGT}>.hex)
        set(MKSIZE ${CMAKE_SIZE})
        set(MKSIZE_ARGS $<TARGET_FILE_NAME:${TGT}>)

        set(SCATTER_FILE ${SCATTER_TPL}.ld)
        set(SCATTER_IN ${SCATTER_FILE}.in)
        set(SCATTER_OUT ${SCATTER_FILE}.cpp)
        set(INCFILE armgnu)
    else()
        message(FATAL_ERROR "Unknown Compiler/Toolset ${CMAKE_C_COMPILER_ID}")
    endif()

    if(NOT CONFIG_PICO)
        # create the real scatter file from the template (*.sct.in)
        message(STATUS ${SCATTER_IN} -> ${SCATTER_OUT})
        configure_file(${SCATTER_IN} ${SCATTER_OUT})
        configure_file(${SCATTER_TPL}.sct.in ${SCATTER_TPL}.sct)
    endif()

    target_compile_definitions(${TGT}
        PUBLIC
            $<UPPER_CASE:${IMAGE}>
            ${MCU}
            FLASHSIZE=${MCU_FLASH_SIZE}
            STACKSIZE=${STACKSIZE}
    )
endif()

# include toolchain specific options/settings
include(${INCFILE})

# set project/target related compiler #define macros
target_compile_definitions(${TGT}
    PUBLIC
        KERNEL_$<IF:$<STREQUAL:${CONFIG_KERNEL},QK>,QK,QV>=1
        $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
        $<$<BOOL:${HEAPSIZE}>:HEAPSIZE=${HEAPSIZE}>
)

add_custom_target(${TGT}Hex ALL
    COMMAND ${MKHEX} ${MKHEX_ARGS}
    COMMAND ${MKSIZE} ${MKSIZE_ARGS}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Create HEX file"
    VERBATIM
)
add_dependencies(${TGT}Hex ${TGT})

findParFile(RESULT PARAM_IN
            FILENAME ${PARAMNAME}
)
findParFile(RESULT PROFILE_IN
            FILENAME ${PROFILENAME}
)

if(PARAM_IN AND PROFILE_IN)
    copyParFile(RESULT PARAM_OUT
                IN ${PARAM_IN}
                TEMPLATE ${PARAMNAME}
    )
    copyParFile(RESULT PROFILE_OUT
                IN ${PROFILE_IN}
                TEMPLATE ${PROFILENAME}
    )

    if(${CMAKE_C_COMPILER_ID} STREQUAL "ARMCC")
        set(_isarmcc TRUE)
    else()
        set(_isarmcc FALSE)
    endif()
    # 2) the hex file with application and eeprom data built from
    #    preceding targets
    getTargetName(TGT_NAME ${TGT})
    add_custom_target(tgtCpl ALL
        COMMAND ${CMAKE_COMMAND}  -DHEXFILE=$<TARGET_NAME:${TGT}> -DPARAM_IN=${CMAKE_CURRENT_BINARY_DIR}/${PARAMNAME}.bin -DPROFILE_IN=${CMAKE_CURRENT_BINARY_DIR}/${PROFILENAME}.bin -DARMCC=${_isarmcc} -P ${CMAKE_CURRENT_LIST_DIR}/createTgtCpl.cmake
        BYPRODUCTS ${TGT_NAME}.cpl.hex
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Create complete hex file for supplier (app code + eeprom data)"
        VERBATIM
        SOURCES ${EEPROM_DATA_FILE}
    )
    add_dependencies(tgtCpl ${TGT}Hex)
    unset(_isarmcc)
endif()
