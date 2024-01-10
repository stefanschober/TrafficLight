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

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Source/cmake ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR} ${HOME_PATH}/cmake)

include(configureProject)
set(QPC_SDK_PATH ${CMAKE_SOURCE_DIR}/Source/qpc)
include(qpc_sdk_import)

# default image/project name is trafficlight
# Give a special name via -DIMAGE=<image>
# the main project
project(${PROJECT_NAME}
    VERSION ${SW_VERSION}
    DESCRIPTION "Trafficlight project"
    LANGUAGES C CXX ASM)

add_subdirectory(Source)

