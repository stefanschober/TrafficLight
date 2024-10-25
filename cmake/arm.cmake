#default port is arm-cm
set(PORT arm-cm)

# no GUI on embedded target
if(CONFIG_GUI)
    message(WARNING "GUI is not supported on embedded targets. Resetting CONFIG_GUI!")
    set(CONFIG_GUI OFF)
endif()

if(CONFIG_PICO)
    message(STATUS "setup Raspberry Pi Pico SDK from ${PICO_SDK_PATH}")
    pico_sdk_init()

    pico_add_extra_outputs(${TGT})

    if(PICO_BOARD STREQUAL pico2)
        set(MCU RP2350)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m33")
    else()
        set(MCU RP2040)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus")
    endif()
else()
    # MCU based settings (size of flash memory)
    if(NOT MCU)
        message(WARNING "No MCU specified. Setting MCU to STM32F091xC!")
        set(MCU STM32F091xC)
    endif()

    if(NOT CONFIG_LIBINIT)
        set(NO_LIBINIT true)
    endif()

    if(MCU MATCHES "STM32F091[^ \t]*C")
        set(MCU_FLASH_SIZE 256)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus")
    elseif(MCU MATCHES "TLE984[2345][^ \t]*")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0")
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
        endif()
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

    # create the real scatter file from the template (*.sct.in)
    message(STATUS ${SCATTER_IN} -> ${SCATTER_OUT})
    configure_file(${SCATTER_IN} ${SCATTER_OUT})

    execute_process(
        COMMAND ${CMAKE_C_COMPILER} -E -I  ${CMAKE_CURRENT_SOURCE_DIR} -P ${SCATTER_FILE}.cpp -o ${SCATTER_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE _ld_ok
    )    
endif()

# set project/target related compiler #define macros
add_custom_target(${TGT}Hex ALL
    COMMAND ${MKHEX} ${MKHEX_ARGS}
    COMMAND ${MKSIZE} ${MKSIZE_ARGS}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Create HEX file"
    VERBATIM
)
add_dependencies(${TGT}Hex ${TGT})
