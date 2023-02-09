# CMake the qpc libraries for different targets
# target may be "Windows", "Posix", "TLE984x"
if(NOT PORT)
    message(FATAL_ERROR "No PORT defined and no PORT could be identified. Please specify -DPORT=<port> (port = win32|posix|arm-cm)")
endif()
set(PORT_DIR ports/${PORT})

IF((CMAKE_SYSTEM_PROCESSOR STREQUAL arm) AND NOT (CONFIG_KERNEL_QV OR CONFIG_KERNEL_QK))
	message(FATAL_ERROR "Target system arm without valid kernel configuration (NO [QV|QK])")
endif()

if(WIN32 OR UNIX)
	if(CONFIG_UNIT_TEST)
		set(PORT_DIR ${PORT_DIR}-qutest)
	elseif(CONFIG_KERNEL_QV)
        set(PORT_DIR ${PORT_DIR}-qv)
    elseif(CONFIG_KERNEL_QK)
        set(PORT_DIR ${PORT_DIR})
	endif()	
elseif(CMAKE_C_COMPILER_ID STREQUAL GNU)
	if(CONFIG_KERNEL_QV)
        set(PORT_DIR ${PORT_DIR}/qv/gnu)
    elseif(CONFIG_KERNEL_QK)
        set(PORT_DIR ${PORT_DIR}/qk/gnu)
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL ARMCC)
	if(CONFIG_KERNEL_QV)
        set(PORT_DIR ${PORT_DIR}/qv/arm)
    elseif(CONFIG_KERNEL_QK)
        set(PORT_DIR ${PORT_DIR}/qk/arm)
    endif()
else()
	message(FATAL_ERROR "Unknown target system (NO [UNIX|WIN32|ARM])")
endif()
message(STATUS "Found port dir - ${PORT_DIR} for port ${PORT}")

# set general defines
target_compile_definitions(${QPLIB}
	PUBLIC
		$<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
   	 	$<$<BOOL:${CONFIG_GUI}>:QWIN_GUI>
   	 	$<$<BOOL:${USE_GTK}>:USE_GTK>
	    $<$<BOOL:${CONFIG_QSPY}>:Q_SPY>
	    $<$<BOOL:${CONFIG_UNIT_TEST}>:Q_UTEST>
        $<$<BOOL:${USE_GTK}>:USE_GTK>
)

target_compile_options(${QPLIB}
    PRIVATE
        $<$<BOOL:${CONFIG_VERBOSE}>:-v>
)

set(_inc include src)
target_include_directories(${QPLIB} PUBLIC ${_inc})

add_subdirectory(src)
if(PORT_DIR)
    add_subdirectory(${PORT_DIR})
endif()

set(_src include/qstamp.c)
target_sources(${QPLIB} PRIVATE ${_src})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

