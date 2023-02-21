if(CONFIG_CPPCHECK)
    set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
    find_program(CPPCHECK NAMES cppcheck cppcheck.exe)
    find_package(Python COMPONENTS Interpreter REQUIRED)

    function(setup_cppcheck target)
        if(CPPCHECK)
            add_custom_command(TARGET ${target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "running cppcheck..."
                COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/patch_compile_commands_json.py
                COMMAND ${CPPCHECK} --template=gcc
                    --cppcheck-build-dir=${CMAKE_BINARY_DIR}
                    --check-library
                    --enable=style,performance,portability,information
                    --language=c
                    --max-ctu-depth=4
                    --std=c11
                    $<$<BOOL:CONFIG_CHECKMISRA>:--addon=${CMAKE_SOURCE_DIR}/misra.json>
                    $<$<BOOL:WIN32>:-D__WIN32__=1>
                    $<$<BOOL:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:-D__GNUC__=1>
                    --project=compile_commands.json
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Perform ccpcheck static analysis"
                COMMAND_EXPAND_LISTS
                VERBATIM
            )
        endif()
    endfunction()
else()
    function(setup_cppcheck target)
    endfunction()
endif()
