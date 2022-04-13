    set(CMAKE_C_FLAGS "--cpu=Cortex-M0 --thumb")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -c -A--cpreproc")


    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR arm)

    set(CMAKE_C_COMPILER armcc)
    set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})
    set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

    set(CMAKE_FIND_ROOT_PATH "${HOME}/.wine/drive_c/Keil")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)