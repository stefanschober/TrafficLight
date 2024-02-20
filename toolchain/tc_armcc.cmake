# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Target definition
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

#---------------------------------------------------------------------------------------
# Set toolchain paths
#---------------------------------------------------------------------------------------
set(TOOLCHAIN arm)

# Set system depended extensions
if(WIN32)
    set(TOOLCHAIN_EXT ".exe" )
else()
    set(TOOLCHAIN_EXT "" )
endif()

# Perform compiler test with static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

set(CMAKE_C_FLAGS "--depend_format=unix --depend_single_line --brief_diagnostics")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --diag_style=gnu --gnu 	--data_reorder --library_type=microlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --apcs=interwork --split_sections -g")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -c -A--predefine=\"__MICROLIB EQU 1\" -A--cpreproc")

#---------------------------------------------------------------------------------------
# Set debug/release build configuration Options
#---------------------------------------------------------------------------------------

# Options for DEBUG build
# -Og   Enables optimizations that do not interfere with debugging.
# -g    Produce debugging information in the operating system’s native format.
set(CMAKE_C_FLAGS_DEBUG "--debug" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "--debug" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "--debug" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "--debug" CACHE INTERNAL "Linker options for debug build type")

# Options for SPY build are identical to DEBUG build
set(CMAKE_C_FLAGS_SPY "${CMAKE_C_FLAGS_DEBUG} -DQ_SPY=1" CACHE INTERNAL "C Compiler options for spy build type")
set(CMAKE_CXX_FLAGS_SPY "${CMAKE_CXX_FLAGS_DEBUG} -DQ_SPY=1" CACHE INTERNAL "C++ Compiler options for spy build type")
set(CMAKE_ASM_FLAGS_SPY "${CMAKE_ASM_FLAGS_DEBUG} -DQ_SPY=1" CACHE INTERNAL "ASM Compiler options for spy build type")
set(CMAKE_EXE_LINKER_FLAGS_SPY "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE INTERNAL "Linker options for spy build type")

# Options for RELEASE build
# -Os   Optimize for size. -Os enables all -O2 optimizations.
# -flto Runs the standard link-time optimizer.
set(CMAKE_C_FLAGS_RELEASE "-Ospace" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Ospace0" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Ospace" CACHE INTERNAL "Linker options for release build type")
# set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Linker options for release build type")

#---------------------------------------------------------------------------------------
# Set compilers
#---------------------------------------------------------------------------------------
set(CMAKE_C_COMPILER ${TOOLCHAIN}cc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER} CACHE INTERNAL "ASM Compiler")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
