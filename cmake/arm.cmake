#default port is arm-cm
set(PORT arm-cm)

if(CONFIG_PICO)
    message(STATUS "setup Raspberry Pi Pico SDK from ${PICO_SDK_PATH}")
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
    # the file ${SCATTER_FILE}.in located in the source directory
    if(NOT DEFINED SCATTER_TPL)
        set(SCATTER_TPL "stm32f091")
    endif()

    if(CMAKE_C_COMPILER_ID STREQUAL "ARMCC")
        find_program(FROMELF
            NAMES fromelf fromelf.exe
            PATHS ${CMAKE_FIND_ROOT_PATH}
            PATH_SUFFIXES bin
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
        set(SCATTER_OUT ${CMAKE_BINARY_DIR}/${SCATTER_FILE})
        set(INCFILE armcc)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        if(NOT CMAKE_OBJCOPY)
            message(FATAL_ERROR "Ooops - the GNU/binutils program 'objcopy' cannot be found!")
        endif()
        find_program(CMAKE_SIZE
            NAMES ${_CMAKE_TOOLCHAIN_PREFIX}size${_CMAKE_TOOLCHAIN_SUFFIX}
            HINTS ${_CMAKE_TOOLCHAIN_LOCATION}
        )
        set(MKHEX ${CMAKE_OBJCOPY})
        set(MKHEX_ARGS -O ihex $<TARGET_FILE_NAME:${TGT}> $<TARGET_NAME:${TGT}>.hex)
        set(MKSIZE ${CMAKE_SIZE})
        set(MKSIZE_ARGS $<TARGET_FILE_NAME:${TGT}>)

        set(SCATTER_FILE ${SCATTER_TPL}.ld)
        set(SCATTER_IN ${SCATTER_FILE}.in)
        set(SCATTER_OUT ${CMAKE_BINARY_DIR}/${SCATTER_FILE}.cpp)
        set(INCFILE armgnu)
    else()
        message(FATAL_ERROR "Unknown Compiler/Toolset ${CMAKE_C_COMPILER_ID}")
    endif()

    if(NOT CONFIG_PICO)
        # create the real scatter file from the template (*.sct.in)
        message(STATUS ${SCATTER_IN} -> ${SCATTER_OUT})
        configure_file(${SCATTER_IN} ${SCATTER_OUT})
    endif()

endif()

# include toolchain specific options/settings
include(${INCFILE})

# set project/target related compiler #define macros
target_compile_definitions(${TGT}
	PUBLIC
        ${MCU}
        FLASHSIZE=${MCU_FLASH_SIZE}
        STACKSIZE=${STACKSIZE}
        $<$<BOOL:${HEAPSIZE}>:HEAPSIZE=${HEAPSIZE}>
        $<$<NOT:$<BOOL:${CONFIG_LIBINIT}>>:NO_LIBINIT>
)

add_custom_target(${TGT}Hex ALL
    COMMAND ${MKHEX} ${MKHEX_ARGS}
    COMMAND ${MKSIZE} ${MKSIZE_ARGS}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Create HEX file"
    VERBATIM
)
add_dependencies(${TGT}Hex ${TGT})

if(PARAMNAME AND PROFILENAME)
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

        # 2) the hex file with application and eeprom data built from
        #    preceding targets
        getTargetName(TGT_NAME ${TGT})
        add_custom_target(tgtCpl ALL
            COMMAND ${CMAKE_COMMAND}
                -DHEXFILE=$<TARGET_NAME:${TGT}>
                -DPARAM_IN=${CMAKE_CURRENT_BINARY_DIR}/${PARAMNAME}.bin
                -DPROFILE_IN=${CMAKE_CURRENT_BINARY_DIR}/${PROFILENAME}.bin
                $<$<STREQUAL:${CMAKE_C_COMPILER_ID}:-DARMCC=ON>
                -P ${CMAKE_CURRENT_LIST_DIR}/createTgtCpl.cmake
            BYPRODUCTS ${TGT_NAME}.cpl.hex
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            COMMENT "Create complete hex file for supplier (app code + eeprom data)"
            VERBATIM
            SOURCES ${EEPROM_DATA_FILE}
        )
        add_dependencies(tgtCpl ${TGT}Hex)
    endif()
endif()