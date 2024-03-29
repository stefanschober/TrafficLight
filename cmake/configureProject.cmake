# TrafficLight QM project configuration options
message("Project configuration and options setting started")

# set up configurable options
option(CONFIG_CPPCHECK   "set to ON, if CPP_CHECK shall be enabled (default: OFF)" OFF)
option(CONFIG_CHECKMISRA "set to ON, if CPP_CHECK shall be enabled with MISRA support (default: OFF)" OFF)

option(CONFIG_GUI        "set to ON, if a Windows (TM) GUI shall be compiled in (default: OFF)" OFF)
option(CONFIG_GTK        "set to ON, if GTK+3 GUI shall be compiled in (default: OFF)" OFF)

option(CONFIG_PICO       "set to ON, if Raspberry Pi Pico system shall be configured. (default: OFF)" OFF)

option(CONFIG_PICOLIB    "set to ON, if the pico standard library shall be used. (default: OFF)" OFF)
option(CONFIG_RASPI_IO   "set to ON, if Raspberry Pi hardware I?O shall be configured. (default: OFF)" OFF)
option(CONFIG_PIGPIO_LIB "set to ON, if the PIGPIO server on the Raspberry Pi shall be used. (default: OFF)" OFF)

option(CONFIG_UNIT_TEST  "set to ON, if Q_UTEST shall be enabled (default: OFF)" OFF)
option(CONFIG_VERBOSE    "set to ON, to set the -v/--verbose option to compiler/linker calls. (default: OFF)" OFF)
option(CONFIG_DEBUG      "set to ON, to enable DEBUG support. (default: ON)" ON)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "The build type (DEBUG, RELEASE, SPY) to build for (default: DEBUG)")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Spy" CACHE STRING "The list of available build configurations for multi config generators")

if(NOT PROJECT_NAME)
    set(PROJECT_NAME "trafficlight" CACHE STRING "The project's name")
    message("set PROJECT_NAME to ('${PROJECT_NAME}') since not specified")
endif()

if(NOT CONFIG_KERNEL)
    set(CONFIG_KERNEL QV CACHE STRING "set to the desired QPC kernel to use - QV, QK or QXK kernel (default: QV)")
endif()

# set binary output directories for targets/build configurations
get_property(isMultiConfiGenerator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfiGenerator)
    unset(CMAKE_BUILD_TYPE)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
else()
    unset(CMAKE_CONFIGURATION_TYPES)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

# check/set Qx real time kernel
set(KERNELS qv qk qxk)
string(TOLOWER "${CONFIG_KERNEL}" KERNEL)
if(NOT (KERNEL IN_LIST KERNELS))
    message(WARNING "Unknown QPC Kernel '${KERNEL}'. Falling back to default kernel (QV)")
    set(CONFIG_KERNEL QV CACHE STRING "set to the desired QPC kernel to use - QV, QK or QXK kernel (default: QV)")
    set(KERNEL qv)
endif()

if(CONFIG_CPPCHECK)
    # set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
    find_program(CPPCHECK NAMES cppcheck cppcheck.exe)
    # set(CPPCHECK ${CMAKE_SOURCE_DIR}/../cppcheck)
    find_package(Python COMPONENTS Interpreter REQUIRED)

    if(CPPCHECK)
        list(APPEND CPPCHECK
            --template=gcc
            --cppcheck-build-dir=${CMAKE_BINARY_DIR}
            --check-library
            --enable=style,performance,portability,information
            --language=c
            --max-ctu-depth=4
            --std=c11
            $<$<BOOL:${CONFIG_CHECKMISRA}>:--addon=${CMAKE_SOURCE_DIR}/misra.json>
            $<$<BOOL:${WIN32}>:-D__WIN32__=1>
            $<$<BOOL:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:-D__GNUC__=1>
            $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-D$<JOIN:$<TARGET_PROPERTY:COMPILE_DEFINITIONS>, -D>>
            $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-I$<JOIN:$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>, -I>>
        )
        message(STATUS "CPPCHEK = ${CPPCHECK}")
        set(CMAKE_C_CPPCHECK ${CPPCHECK})
        set(CMAKE_CXX_CPPCHECK ${CPPCHECK})
    endif()
endif()

if(NOT TARGET)
    set(TARGET "${PROJECT_NAME}" CACHE STRING "The target name (default: PROJECT_NAME)")
endif()
if(NOT IMAGE)
    set(IMAGE "${TARGET}" CACHE STRING "The name in the binary image. (default: TARGET)")
endif()
set(TGT ${TARGET})

if(NOT SW_VERSION)
    set(SW_VERSION "1.0.0" CACHE STRING "Software Version")
endif()
