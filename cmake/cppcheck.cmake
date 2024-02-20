if(CONFIG_CPPCHECK)
    set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
    find_program(CPPCHECK NAMES cppcheck cppcheck.exe)
    # set(CPPCHECK "${CMAKE_SOURCE_DIR}/../cppcheck.exe")

    find_package(Python COMPONENTS Interpreter REQUIRED)
    if(CONFIG_CHECKMISRA)
        if(DEFINED ENV{MSYSTEM_PREFIX})
            set(PREFIX $ENV{MSYSTEM_PREFIX}/share)
        elseif(UNIX)
            set(PREFIX /usr/share)
        endif()
        file(GLOB_RECURSE MISRA_PY
            LIST_DIRECTORIES FALSE
            FOLLOW_SYMLINKS
            ${PREFIX}/misra.py /usr/local/share/misra.py
        )
        file(GLOB_RECURSE MISRA_TXT
            LIST_DIRECTORIES FALSE
            FOLLOW_SYMLINKS
            $ENV{HOME}/misra*.txt
        )
        list(GET MISRA_TXT -1 MISRA_TXT)
        list(GET MISRA_PY 0 MISRA_PY)

        string(REPLACE \\ / MISRA_PY ${MISRA_PY})
        string(REPLACE \\ / MISRA_TXT ${MISRA_TXT})
        configure_file(misra.json.in misra.json NO_SOURCE_PERMISSIONS NEWLINE_STYLE UNIX)
    endif()

    if(CONFIG_CPPCHECK_TGT)
        set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
        function(setup_cppcheck target)
            if(CPPCHECK)
                list(APPEND CPPCHECK
                    --template=gcc
                    --cppcheck-build-dir=${CMAKE_BINARY_DIR}
                    --enable=style,performance,portability,information
                    --language=c
                    --max-ctu-depth=4
                    --std=c11
                    $<$<BOOL:${CONFIG_VERBOSE}>:--verbose>
                    $<$<BOOL:${CONFIG_CHECKMISRA}>:--addon=misra.json>
                    $<$<BOOL:${WIN32}>:-D__WIN32__=1>
                    $<$<BOOL:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:-D__GNUC__=1>
                    # $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-D$<JOIN:$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>, -D>>
                    $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-I$<JOIN:$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>, -I>>
                    --output-file=${target}.cppcheck
                    --project=compile_commands.json
                )
                add_custom_command(TARGET ${target}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "running cppcheck..."
                    COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/patch_compile_commands_json.py
                    COMMAND ${CPPCHECK}
                    COMMAND ${CMAKE_COMMAND} -E cat ${target}.cppcheck
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Perform ccpcheck static analysis"
                    COMMAND_EXPAND_LISTS
                    VERBATIM
                )
            endif()
        endfunction()
    else()
        function(setup_cppcheck target)
            if(CPPCHECK)
                list(APPEND CPPCHECK
                    --template=gcc
                    --cppcheck-build-dir=${CMAKE_BINARY_DIR}
                    --enable=style,performance,portability,information
                    --language=c
                    --max-ctu-depth=4
                    --std=c11
                    $<$<BOOL:${CONFIG_VERBOSE}>:--verbose>
                    $<$<BOOL:${CONFIG_CHECKMISRA}>:--addon=misra.json>
                    $<$<BOOL:${WIN32}>:-D__WIN32__=1>
                    $<$<BOOL:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:-D__GNUC__=1>
                    # $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-D$<JOIN:$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>, -D>>
                    $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-I$<JOIN:$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>, -I>>
                )
                set_target_properties(${target}
                    PROPERTIES
                        C_CPPCHECK "${CPPCHECK}"
                        CXX_CPPCHECK "{CPPCHECK}"
                )
            endif()
        endfunction()
    endif()
else()
    function(setup_cppcheck target)
        # message(WARNING "CONF_CPPCHECK unset; cppcheck functionality disabled!")
    endfunction()
endif()
