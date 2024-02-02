# TrafficLight QM project root CMakeLists.txt

# uncomment to enable ccache
#find_program(CCACHE_PROGRAM ccache ccache.exe)
#if(CCACHE_PROGRAM)
#    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
#    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
#endif()

# set C-C++ standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(cppcheck)
include(functions)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    include(CheckPIESupported)
    check_pie_supported()
endif()

# the project target(s)
add_executable(${TGT} "")
set_property(TARGET ${TGT} PROPERTY POSITION_INDEPENDENT_CODE FALSE)
set_property(TARGET ${TGT} PROPERTY OUTPUT_NAME ${IMAGE})

# include target specific definitions (set compiler options etc.)
if(DEFINED PORT AND PORT MATCHES arm)
    include(arm)
elseif(DEFINED PORT AND PORT MATCHES posix)
    include(unix)
elseif(DEFINED PORT AND PORT MATCHES win32)
    include(win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL Generic AND CMAKE_SYSTEM_PROCESSOR MATCHES [Aa][Rr][Mm])
	include(arm)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux OR UNIX)
    include(unix)
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows OR WIN32)
    include(win32)
else()
	message(FATAL_ERROR "Unknown target port (NOT [UNIX|WIN32|ARM])!")
endif()

# add subdirectories with source/header files
add_subdirectory(BSP)
add_subdirectory(QM)

include(${QPC_SDK_PATH}/qpc_sdk_init.cmake)
set(QPC_PROJECT qpc_${PROJECT_NAME})
set(QPC_CFG_KERNEL ${CONFIG_KERNEL})
set(QPC_QFG_UNIT_TEST ${CONFIG_UNIT_TEST})
set(QPC_CFG_GUI ${CONFIG_GUI})
set(QPC_CFG_VERBOSE ${CONFIG_VERBOSE})
set(QPC_CFG_PORT ${PORT})
qpc_sdk_init()

target_link_libraries(${TGT} PRIVATE qpc)

# set global options/defines
include(globalOptions)

setup_cppcheck(${TGT})

# show configuration results
message(STATUS
"========================================================
Configured project ${PROJECT_NAME} for ${PORT}
 Project Settings:
    PROJECT_NAME                = ${PROJECT_NAME}
    TARGET                      = ${TARGET}
    IMAGE                       = ${IMAGE}
    SW_VERSION                  = ${SW_VERSION}
    PORT                        = ${PORT}
-- ========================================================
"
)

# show target configuration, if CONFIG_VERBOSE is set to ON
if(CONFIG_VERBOSE)
    get_target_property(TGT_COMPILE_OPTS ${TGT} COMPILE_OPTIONS)
    get_target_property(TGT_COMPILE_DEFS ${TGT} COMPILE_DEFINITIONS)
    get_target_property(TGT_INCLUDES ${TGT} INCLUDE_DIRECTORIES)
    get_target_property(TGT_LINK_OPTS ${TGT} LINK_OPTIONS)
    get_target_property(TGT_LINK_LIBS ${TGT} LINK_LIBRARIES)
    get_target_property(TGT_SOURCES ${TGT} SOURCES)

    message(STATUS
"========================================================
  Configuration options set:
    CONFIG_GUI                  = ${CONFIG_GUI}
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

  Target information for target ${TGT}
    TGT_COMPILE_OPTS            = ${TGT_COMPILE_OPTS}
    TGT_COMPILE_DEFS            = ${TGT_COMPILE_DEFS}
    TGT_INCLUDES                = ${TGT_INCLUDES}
    TGT_LINK_OPTS               = ${TGT_LINK_OPTS}
    TGT_LINK_LIBS               = ${TGT_LINK_LIBS}
    TGT_SOURCES                 = ${TGT_SOURCES}

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

    CMAKE_BUILD_TYPE            = ${CMAKE_BUILD_TYPE}
    CMAKE_C_COMPILER            = ${CMAKE_C_COMPILER}
    CMAKE_C_COMPILER_ID         = ${CMAKE_C_COMPILER_ID}
    CMAKE_C_COMPILER_VERSION    = ${CMAKE_C_COMPILER_VERSION}
    CMAKE_C_FLAGS               = ${CMAKE_C_FLAGS}

    CMAKE_CXX_COMPILER          = ${CMAKE_CXX_COMPILER}
    CMAKE_CXX_FLAGS             = ${CMAKE_CXX_FLAGS}

    CMAKE_ASM_COMPILER          = ${CMAKE_ASM_COMPILER}
    CMAKE_ASM_FLAGS             = ${CMAKE_ASM_FLAGS}
-- ========================================================
"
)
endif()
