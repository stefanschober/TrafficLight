# qpc/src
add_subdirectory(qf)
if(NOT (WIN32 OR UNIX))
    if(CONFIG_KERNEL_QK)
        add_subdirectory(qk)
    endif()

    if(CONFIG_KERNEL_QV)
        add_subdirectory(qv)
    endif()
endif()

if(CONFIG_QSPY)
    add_subdirectory(qs)
endif()
