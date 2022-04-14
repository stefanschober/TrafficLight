# USC2 QM project root CMakeLists.txt

# use a recent CMake version
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(VERSION 3.13)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    cmake_policy(SET CMP0083 NEW)
endif()

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
    set(CMAKE_MODULE_PATH "${HOME_PATH}/cmake;${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
else()
    set(CMAKE_MODULE_PATH $ENV{HOME}/cmake;${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH})
endif()
# message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

# set up configurable options
option(CONFIG_QSPY       "set to ON, if QSPY monitoring shall be compiled in (default: OFF)" OFF)
option(CONFIG_UNIT_TEST  "set to ON, if Q_UTEST shall be enabled (default: OFF)" OFF)
option(CONFIG_CPPCHECK   "set to ON, if CPP_CHECK shall be enabled (default: OFF)" OFF)
option(CONFIG_CHECKMISRA "set to ON, if CPP_CHECK shall be enabled with MISRA support (default: OFF)" OFF)
option(CONFIG_GUI        "set to ON, if a Windows (TM) GUI shall be compiled in (default: OFF)" OFF)
option(CONFIG_KERNEL_QV  "set to ON, if the QV micro kernel shall be configured. Leave OFF for host based configurations. (default: OFF)" OFF)
option(CONFIG_KERNEL_QK  "set to ON, if the QK micro kernel shall be configured. Leave OFF for host based configurations. (default: OFF)" OFF)
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
if((NOT (CONFIG_KERNEL_QV OR CONFIG_KERNEL_QK)) OR (CONFIG_KERNEL_QV AND CONFIG_KERNEL_QK))
    if(CONFIG_KERNEL_QK)
        message(WARNING "Only one KERNEL (out of QV or QK) shall be configured! Accepting QK kernel.")
    else()
        message(WARNING "No KERNEL (QV or QK) is configured! Falling back to QK kernel.")
    endif()

    set(CONFIG_KERNEL_QV OFF CACHE BOOL "set to ON, if the QV micro kernel shall be configured. Leave OFF for host based configurations. (default: OFF)" FORCE)
    set(CONFIG_KERNEL_QK ON  CACHE BOOL "set to ON, if the QK micro kernel shall be configured. Leave OFF for host based configurations. (default: OFF)" FORCE)
endif()

if(CONFIG_KERNEL_QK)
    set(QPLIB "qpclib-qk" CACHE STRING "The QPC library for the QV real time kernel" FORCE)
else()
    set(QPLIB "qpclib-qv" CACHE STRING "The QPC library for the QV real time kernel" FORCE)
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

# the project target
add_executable(${TGT} "")
if(IMAGE)
    set_property(TARGET ${TGT} PROPERTY OUTPUT_NAME ${IMAGE})
endif()
setup_cppcheck(${TGT})

add_library(${QPLIB} STATIC "")
target_link_libraries(${TGT} PRIVATE ${QPLIB})

# set general defines
target_compile_definitions(${TGT}
	PRIVATE
	    $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
)

target_compile_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

target_link_options(${TGT}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

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
else()
	message(FATAL_ERROR "Unknown target port (NOT [UNIX|WIN32|ARM])!")
endif()

# set position independent code compile/link parameters
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    check_pie_supported()
endif()
set_property(TARGET ${TGT} PROPERTY POSITION_INDEPENDENT_CODE FALSE)
set_property(TARGET ${QPLIB} PROPERTY POSITION_INDEPENDENT_CODE FALSE)

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
add_subdirectory(qpc)
add_subdirectory(BSP)
add_subdirectory(QM)

# show configuration results
get_target_property(TGT_COMPILE_OPTS ${TGT} COMPILE_OPTIONS)
get_target_property(TGT_COMPILE_DEFS ${TGT} COMPILE_DEFINITIONS)
get_target_property(TGT_INCLUDES ${TGT} INCLUDE_DIRECTORIES)
get_target_property(TGT_LINK_OPTS ${TGT} LINK_OPTIONS)
get_target_property(TGT_LINK_LIBS ${TGT} LINK_LIBRARIES)
get_target_property(TGT_SOURCES ${TGT} SOURCES)
message(STATUS
"Configured project ${PROJECT_NAME} for ${PORT}
 Configuration options set:
    PROJECT_NAME                = ${PROJECT_NAME}
    TARGET                      = ${TARGET}
    IMAGE                       = ${IMAGE}
    SW_VERSION                  = ${SW_VERSION}
    
    CONFIG_GUI                  = ${CONFIG_GUI}
    CONFIG_CONSOLE              = ${CONFIG_CONSOLE}
    CONFIG_QSPY                 = ${CONFIG_QSPY}
    CONFIG_UNIT_TEST            = ${CONFIG_UNIT_TEST}
    CONFIG_CPPCHECK             = ${CONFIG_CPPCHECK}
    CONFIG_CHECKMISRA           = ${CONFIG_CHECKMISRA}
    CMAKE_C_CPPCHECK            = ${CMAKE_C_CPPCHECK}
    
    CONFIG_KERNEL_QV            = ${CONFIG_KERNEL_QV}
    CONFIG_KERNEL_QK            = ${CONFIG_KERNEL_QK}
    QPLIB                       = ${QPLIB}

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

# Target information for target ${TGT}
#    TGT_COMPILE_OPTS            = ${TGT_COMPILE_OPTS}
#    TGT_COMPILE_DEFS            = ${TGT_COMPILE_DEFS}
#    TGT_INCLUDES                = ${TGT_INCLUDES}
#    TGT_LINK_OPTS               = ${TGT_LINK_OPTS}
#    TGT_LINK_LIBS               = ${TGT_LINK_LIBS}
#    TGT_SOURCES                 = ${TGT_SOURCES}"
#)
