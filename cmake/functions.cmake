# miscellaneous functions for various use cases

# functions to work with .par files
function(findParFile)
    set(options)
    set(oneValueArgs RESULT FILENAME)
    set(multiValueArgs)
    cmake_parse_arguments(FPF "${options}" "${oneValueArgs}"  "${multiValueArgs}" ${ARGN})
    if (NOT (FPF_RESULT AND FPF_FILENAME))
        message(FATAL_ERROR "usage: findParFile(RESULT <resultVar> FILENAME <parfile name>)")
    endif()

    set(_fnx ${FPF_FILENAME})
    string(TOUPPER ${_fnx} _fnu)
    string(TOLOWER ${_fnx} _fnl)

    find_file(${FPF_RESULT}
        NAMES
            ${_fnx} ${_fnx}.bin
            ${_fnu} ${_fnu}.BIN
            ${_fnl} ${_fnl}.bin
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_PROJECT_SOURCE_DIR}
        NO_DEFAULT_PATH
        NO_PACKAGE_ROOT_PATH
        NO_CMAKE_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
endfunction()

function(copyParFile)
    set(options)
    set(oneValueArgs RESULT IN TEMPLATE)
    set(multiValueArgs)

    cmake_parse_arguments(CPF "${options}" "${oneValueArgs}"  "${multiValueArgs}" ${ARGN})
    if (NOT (CPF_RESULT AND CPF_IN AND CPF_TEMPLATE))
        message(FATAL_ERROR "usage: copyParFile(RESULT <resultVar> IN <inFile> TEMPLATE <fileName template>)")
    endif()

    set(_out ${CMAKE_CURRENT_BINARY_DIR}/${CPF_TEMPLATE}.bin)
    if(NOT EXISTS ${_out})
        file(GENERATE OUTPUT ${_out} INPUT ${CPF_IN})
    endif()
    
    set(${CPF_RESULT} ${_out} PARENT_SCOPE)
endfunction()

function(getTargetName _name _tgt)
    get_target_property(_n ${_tgt} OUTPUT_NAME)
    get_target_property(_p ${_tgt} PREFIX)
    
    if(NOT _p)
        set(_p "")
    endif()
    if(NOT _n)
        get_target_property(_n ${_tgt} NAME)
        if(NOT _n)
            set(_n ${CMAKE_PROJECT_NAME})
        endif()
    endif()
    
    set(${_name} "${_p}${_n}" PARENT_SCOPE)
endfunction()

