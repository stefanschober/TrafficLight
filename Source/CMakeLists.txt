# USC2 QM project root CMakeLists.txt

# use a recent CMake version
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
cmake_policy(VERSION 3.23...3.28)
cmake_policy(SET CMP0083 NEW)
cmake_policy(SET CMP0105 NEW)
cmake_policy(SET CMP0116 NEW)
cmake_policy(SET CMP0128 NEW)

# first of all protect against in-source builds
get_filename_component(_srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(_bindir "${CMAKE_BINARY_DIR}" REALPATH)

if(${_srcdir} STREQUAL ${_bindir})
    message(FATAL_ERROR "  FATAL: In-source builds are not allowed!
         You should create a separate directory for build files.")
else()
    unset(_srcdir)
    unset(_bindir)
endif()

# update the module search path
if(MSYS OR WINDOWS OR WIN32)
    string(REPLACE "\\" "/" HOME_PATH "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
    set(CMAKE_MODULE_PATH ${HOME_PATH}/cmake;${CMAKE_SOURCE_DIR}/cmake;${CMAKE_SOURCE_DIR};${CMAKE_MODULE_PATH})
else()
    set(CMAKE_MODULE_PATH $ENV{HOME}/cmake;${CMAKE_SOURCE_DIR}/cmake;${CMAKE_SOURCE_DIR};${CMAKE_MODULE_PATH})
endif()
# message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

# set up configurable options
option(CONFIG_QSPY       "set to ON, if QSPY monitoring shall be compiled in (default: OFF)" OFF)
option(CONFIG_UNIT_TEST  "set to ON, if Q_UTEST shall be enabled (default: OFF)" OFF)
option(CONFIG_CPPCHECK   "set to ON, if CPP_CHECK shall be enabled (default: OFF)" OFF)
option(CONFIG_CPPCHECK_TGT "set to ON, to generate a separate CPPCHECK target (default: OFF)" OFF)
option(CONFIG_CHECKMISRA "set to ON, if CPP_CHECK shall be enabled with MISRA support (default: OFF)" OFF)
option(CONFIG_GUI        "set to ON, if a Windows (TM) GUI shall be compiled in (default: OFF)" OFF)
option(CONFIG_PICOLIB    "set to ON, if the pico standard library shall be used. (default: OFF)" OFF)
option(CONFIG_RASPI      "set to ON, if Raspberry Pi Linux system shall be configured. (default: OFF)" OFF)
option(CONFIG_RASPI_IO   "set to ON, if Raspberry Pi hardware I?O shall be configured. (default: OFF)" OFF)
option(CONFIG_PIGPIO     "set to ON, if the PIGPIO server on the Raspberry Pi shall be used. (default: OFF)" OFF)
option(CONFIG_CONSOLE   "set to ON, if Linux console output shall be configured. (default: OFF)" OFF)
option(CONFIG_PICO      "set to ON, if Raspberry Pi Pico system shall be configured. (default: OFF)" OFF)
option(CONFIG_PICO_CMSIS "set to ON, if Raspberry Pi Pico system shall use CMSIS APIs. (default: ON)" ON)
option(CONFIG_CORTEX_M0 "set to ON, if an ARM Cortex-M0 target system shall be configured. (default: OFF)" OFF)
option(CONFIG_CORTEX_M0plus "set to ON, if an ARM Cortex-M0+ target system shall be configured. (default: OFF)" OFF)
option(CONFIG_CORTEX_M3 "set to ON, if an ARM Cortex-M3 target system shall be configured. (default: OFF)" OFF)
option(CONFIG_CORTEX_M4 "set to ON, if an ARM Cortex-M4 target system shall be configured. (default: OFF)" OFF)
option(CONFIG_VERBOSE   "set to ON, to set the -v/--verbose option to compiler/linker calls. (default: OFF)" OFF)
option(CONFIG_DEBUG     "set to ON, to enable DEBUG support. (default: ON)" ON)
set(CONFIG_KERNEL QV CACHE STRING "set to the desired QPC kernel to use - QV, QK or QXK kernel (default: QV)")

if(CONFIG_UNIT_TEST)
	set(CONFIG_QSPY ON CACHE BOOL "set to ON, if QSPY monitoring shall be compiled in (default: OFF)" FORCE)
endif()

# setup Raspberry Pi Pico SDK
if(CONFIG_PICO)
    find_path(PICO_SDK_PATH pico_sdk_init.cmake
              PATHS ~ /opt /var /usr/lib /
              PATH_SUFFIXES pico pico-sdk pico/pico-sdk)

    if(PICO_SDK_PATH)
        message(STATUS "setup Raspberry Pi Pico SDK from ${PICO_SDK_PATH}")
        set(CMAKE_BUILD_TYPE "Debug")
        set(PICO_BOARD pico)
        set(PICO_BARE_METAL FALSE)
        set(PICO_PLATFORM rp2040)

        # include must happen before project(...)
        include(pico_sdk_import)
    else()
        message(FATAL "PICO_SDK_PATH not found!")
    endif()
elseif(CONFIG_DEBUG)
    set(CMAKE_BUILD_TYPE "Debug")
else()
    set(CMAKE_BUILD_TYPE "Release")
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

        # set(CMAKE_C_CPPLINT ${CPPCHECK})
        # set(CMAKE_CXX_CPPLINT ${CPPCHECK})

        # set(CMAKE_C_CLANG_TIDY ${CPPCHECK})
        # set(CMAKE_CXX_CLANG_TIDY ${CPPCHECK})

        # set(CMAKE_C_NCLUDE_WHAT_YOU_USE ${CPPCHECK})
        # set(CMAKE_CXX_NCLUDE_WHAT_YOU_USE ${CPPCHECK})

    endif()
endif()

if(NOT PROJECT_NAME AND NOT TARGET)
    set(PROJECT_NAME "trafficlight")
    set(TARGET ${PROJECT_NAME})
elseif(NOT PROJECT_NAME)
    set(PROJECT_NAME ${TARGET})
elseif(NOT TARGET)
    set(TARGET ${PROJECT_NAME})
endif()
if(NOT IMAGE)
    set(IMAGE ${TARGET})
endif()
set(TGT ${TARGET})

if(NOT SW_VERSION)
    set(SW_VERSION "1.0.0" CACHE STRING "Software Version")
endif()

# default image/project name is usc2
# Give a special name via -DIMAGE=<image>
# the main project
project(${PROJECT_NAME}
    VERSION ${SW_VERSION}
    DESCRIPTION "Trafficlight project"
    LANGUAGES C ASM)

# enable ccache
find_program(CCACHE_PROGRAM ccache ccache.exe)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# update the module search path
if(CMAKE_HOST_WIN32)
    string(REPLACE "\\" "/" HOME_PATH "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
    set(CMAKE_MODULE_PATH "${HOME_PATH}/cmake;${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
else()
    set(CMAKE_MODULE_PATH $ENV{HOME}/cmake;${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH})
endif()
# message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

# setup Raspberry Pi Pico SDK
if(CONFIG_PICO)
    message(STATUS "setup Raspberry Pi Pico SDK")
    set(CMAKE_BUILD_TYPE Debug)
    set(PICO_BOARD pico)
    set(PICO_BARE_METAL FALSE)
    set(PICO_PLATFORM rp2040)
    include(pico_sdk_import)
endif()

# set C-C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(cppcheck)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    include(CheckPIESupported)
endif()

# set up global C/CXX/ASM flags
if(CONFIG_PICOLIB)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} --specs=picolibc.specs")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif()

# the project target(s)
add_executable(${TGT} "")
if(IMAGE)
    set_property(TARGET ${TGT} PROPERTY OUTPUT_NAME ${IMAGE})
endif()
setup_cppcheck(${TGT})

# set the default file names for parameter and profile data
# the files are in binary format and need to have
# file name extension of ".bin"
set(PARAMNAME   "USC2_Parameter")
set(PROFILENAME "USC2_Profile")

# include target specific definitions (set compiler options etc.)
if(CMAKE_SYSTEM_NAME STREQUAL Linux OR UNIX)
    include(unix)
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows OR WIN32)
    include(win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL Generic AND CMAKE_SYSTEM_PROCESSOR MATCHES [Aa][Rr][Mm])
	include(arm)
#if(CONFIG_PICO)
#    add_executable(systicktst systicktst.c)
#    pico_enable_stdio_usb(systicktst TRUE)
#    pico_enable_stdio_uart(systicktst FALSE)

#    pico_add_extra_outputs(systicktst)
#    target_link_libraries(systicktst pico_stdlib pico_time hardware_exception cmsis_core)

#    target_compile_definitions(systicktst
#        PUBLIC
#            PICO_CMSIS_RENAME_EXCEPTIONS=1
#            PICO_NO_RAM_VECTOR_TABLE=1
#    )

#    target_compile_options(systicktst
#        PUBLIC
#            -O0
#            -g3
#    )

#    pico_set_binary_type(systicktst no_flash)
#    pico_set_binary_type(systicktst default)
#endif()

else()
	message(FATAL_ERROR "Unknown target port (NOT [UNIX|WIN32|ARM])!")
endif()

# set general defines
target_compile_definitions(${TGT}
	PRIVATE
	    $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
        $<$<BOOL:${CONFIG_GUI}>:QWIN_GUI>
        TARGET=${TGT}
)

target_compile_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

target_link_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

# set position independent code compile/link parameters
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    check_pie_supported()
endif()
set_property(TARGET ${TGT} PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# define function to output list of modules
set (MODLIST ${CMAKE_BINARY_DIR}/modlist.txt)
file(WRITE ${MODLIST} "Module list for Project ${PROJECT_NAME} (${PORT})\n")
file(APPEND ${MODLIST} "Using C Compiler ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION} (${CMAKE_C_COMPILER_ID})\n")
function(listModules PATH)
  file(APPEND ${MODLIST} ${PATH} ":\n")
  foreach(_mod IN LISTS ARGN)
    file(APPEND ${MODLIST} "    - ${_mod}\n")
  endforeach()
endfunction()
function(listIncludes PATH)
#  file(APPEND ${MODLIST} "Includes - ${PATH}:\n")
#  foreach(_inc IN LISTS ARGN)
#    file(APPEND ${MODLIST} "    - ${_inc}\n")
#  endforeach()
endfunction()

# add subdirectories with source/header files
add_subdirectory(BSP)
add_subdirectory(QM)

if(NOT (DEFINED QPC_DIR OR DEFINED ENV{QPC_DIR}))
    find_path(QPC_DIR
        PATHS "$ENV{HOME}/Projects" "$ENV{HOME}" ENV HOME "/usr/lib" "/opt"
        NAMES "include/qpc.h" "qpc.qm"
        PATH_SUFFIXES "qpc" "Qpc" "QPC"
        REQUIRED
    )
elseif((NOT DEFINED QPC_DIR) AND DEFINED ENV{QPC_DIR})
    set(QPC_DIR $ENV{QPC_DIR})
endif()

if(QPC_DIR)
    set(QPC_PROJECT qpc_${PROJECT_NAME})
    set(QPC_CFG_KERNEL ${CONFIG_KERNEL})
    set(QPC_CFG_QSPY ${CONFIG_QSPY})
    set(QPC_QFG_UNIT_TEST ${CONFIG_UNIT_TEST})
    set(QPC_CFG_GUI ${CONFIG_GUI})

    add_subdirectory(${QPC_DIR} ${CMAKE_BINARY_DIR}/qpc)
    target_link_libraries(${TGT} PRIVATE qpc)
else()
    message(FATAL_ERROR "QPC_DIR not found!")
endif()

# show configuration results
message(STATUS
"========================================================
Configured project ${PROJECT_NAME} for ${PORT}
 Configuration options set:
    PROJECT_NAME                = ${PROJECT_NAME}
    TARGET                      = ${TARGET}
    IMAGE                       = ${IMAGE}
    SW_VERSION                  = ${SW_VERSION}
    
    PORT                        = ${PORT}
    CONFIG_GUI                  = ${CONFIG_GUI}
    CONFIG_CONSOLE              = ${CONFIG_CONSOLE}
    CONFIG_QSPY                 = ${CONFIG_QSPY}
    CONFIG_UNIT_TEST            = ${CONFIG_UNIT_TEST}
    CONFIG_CPPCHECK             = ${CONFIG_CPPCHECK}
    CONFIG_CHECKMISRA           = ${CONFIG_CHECKMISRA}
    CMAKE_C_CPPCHECK            = ${CMAKE_C_CPPCHECK}
    
    CONFIG_KERNEL               = ${CONFIG_KERNEL}
    QPC_DIR                     = ${QPC_DIR}

    CONFIG_CORTEX_M0            = ${CONFIG_CORTEX_M0}
    CONFIG_CORTEX_M0plus        = ${CONFIG_CORTEX_M0plus}
    CONFIG_CORTEX_M3            = ${CONFIG_CORTEX_M3}
    CONFIG_CORTEX_M4            = ${CONFIG_CORTEX_M4}
    CONFIG_PICO                 = ${CONFIG_PICO}
    MCU                         = ${MCU}

    CONFIG_RASPI                = ${CONFIG_RASPI}
    CONFIG_RASPI_IO             = ${CONFIG_RASPI_IO}
    CONFIG_PIGPIO               = ${CONFIG_PIGPIO}

    CONFIG_DEBUG                = ${CONFIG_DEBUG}
    
 System information:
    CMAKE_VERSION               = ${CMAKE_VERSION}
    CMAKE_CROSSCOMPILING        = ${CMAKE_CROSSCOMPILING}
    CMAKE_HOST_SYSTEM           = ${CMAKE_HOST_SYSTEM}
    CMAKE_HOST_SYSTEM_NAME      = ${CMAKE_HOST_SYSTEM_NAME}
    CMAKE_HOST_LINUX            = ${CMAKE_HOST_LINUX}
    CMAKE_HOST_UNIX             = ${CMAKE_HOST_UNIX}
    CMAKE_HOST_WIN32            = ${CMAKE_HOST_WIN32}
    CMAKE_SYSTEM                = ${CMAKE_SYSTEM}
    CMAKE_SYSTEM_NAME           = ${CMAKE_SYSTEM_NAME}
    CMAKE_SYSTEM_PROCESSOR      = ${CMAKE_SYSTEM_PROCESSOR}
    WIN32                       = ${WIN32}
    MSYS                        = ${MSYS}
    MINGW                       = ${MINGW}
    UNIX                        = ${UNIX}
    LINUX                       = ${LINUX}

    CMAKE_C_COMPILER            = ${CMAKE_C_COMPILER}
    CMAKE_C_COMPILER_ID         = ${CMAKE_C_COMPILER_ID}
    CMAKE_C_COMPILER_VERSION    = ${CMAKE_C_COMPILER_VERSION}
    CMAKE_C_FLAGS               = ${CMAKE_C_FLAGS}

    CMAKE_CXX_COMPILER          = ${CMAKE_CXX_COMPILER}
    CMAKE_CXX_FLAGS             = ${CMAKE_CXX_FLAGS}

    CMAKE_ASM_COMPILER          = ${CMAKE_ASM_COMPILER}
    CMAKE_ASM_FLAGS             = ${CMAKE_ASM_FLAGS}"
)

# show target configuration, if CONFIG_VERBOSE is set to ON
if(CONFIG_VERBOSE)
    message(STATUS
"========================================================
Target information for target ${TGT}
    TGT_COMPILE_OPTS            = ${TGT_COMPILE_OPTS}
    TGT_COMPILE_DEFS            = ${TGT_COMPILE_DEFS}
    TGT_INCLUDES                = ${TGT_INCLUDES}
    TGT_LINK_OPTS               = ${TGT_LINK_OPTS}
    TGT_LINK_LIBS               = ${TGT_LINK_LIBS}
    TGT_SOURCES                 = ${TGT_SOURCES}"
)
endif()
