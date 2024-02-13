# TrafficLight QM project root CMakeLists.txt

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
else()
    set(HOME_PATH $ENV{HOME})
endif()
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Source/cmake ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR} ${HOME_PATH}/cmake)


# setup Raspberry Pi Pico SDK
if(CONFIG_PICO)
    # include must happen before project(...)
    if(NOT PICO_SDK_PATH) # check for already installed Pico SDK
        if(EXISTS $ENV{HOME}/Projects/pico-sdk)
            set(PICO_SDK_PATH "$ENV{HOME}/Projects/pico-sdk")
        elseif(DEFINED ENV{PICO_SDK_PATH})
            set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
        endif()
    endif()
    set(PICO_BOARD pico)
    set(PICO_BARE_METAL FALSE)
    set(PICO_PLATFORM rp2040)

    include(pico_sdk_import)

    if(NOT PICO_SDK_PATH)
	    unset(PICO_BOARD)
    	unset(PICO_BARE_METAL)
    	unset(PICO_PLATFORM)
        set(CONFIG_PICO OFF)
        message(FATAL "PICO_SDK_PATH not found!")
    endif()
endif()

# include general project config & import qpc
include(configureProject)
set(QPC_SDK_PATH ${CMAKE_SOURCE_DIR}/Source/qpc)
include(qpc_sdk_import)

# default image/project name is trafficlight
# Give a special name via -DIMAGE=<image>
# the main project
project(${PROJECT_NAME}
    VERSION ${SW_VERSION}
    DESCRIPTION "Trafficlight project"
    LANGUAGES C CXX ASM
)

if(NOT CMAKE_C_FLAGS_SPY)
    foreach(LANG IN ITEMS C CXX ASM)
        set(CMAKE_${LANG}_FLAGS_SPY ${CMAKE_${LANG}_FLAGS_DEBUG})
    endforeach()
endif()

add_subdirectory(Source)
