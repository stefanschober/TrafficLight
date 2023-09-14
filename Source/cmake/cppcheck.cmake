if(CONFIG_CPPCHECK)
    # set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
    find_program(CPPCHECK NAMES cppcheck cppcheck.exe)
    find_package(Python COMPONENTS Interpreter REQUIRED)

    function(setup_cppcheck)
        if(CPPCHECK)
            set(CPPCHECK ${CMAKE_SOURCE_DIR}/../cppcheck)
            #list(APPEND CPPCHECK
            #    "--template=gcc"
            #    "--cppcheck-build-dir=${CMAKE_BINARY_DIR}"
            #    --check-library
            #    --enable=style,performance,portability,information
            #    --language=c
            #    --max-ctu-depth=4
            #    --std=c11
            #    $<$<BOOL:${CONFIG_CHECKMISRA}>:--addon=${CMAKE_SOURCE_DIR}/misra.json>
            #    $<$<BOOL:${WIN32}>:-D__WIN32__=1>
            #    $<$<BOOL:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:-D__GNUC__=1>
            #    $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-D$<JOIN:$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>, -D>>
            #    $<$<BOOL:$<NOT:$<STREQUAL:${CMAKE_GENERATOR},Ninja>>>:-I$<JOIN:$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>, -I>>
            #)
            message(STATUS "CPPCHEK = ${CPPCHECK}")
            set(CMAKE_C_CPPCHECK ${CPPCHECK})
            set(CMAKE_CXX_CPPCHECK ${CPPCHECK})

            # set(CMAKE_C_CPPLINT ${CPPCHECK})
            # set(CMAKE_CXX_CPPLINT ${CPPCHECK})

            # set(CMAKE_C_CLANG_TIDY ${CPPCHECK})
            # set(CMAKE_CXX_CLANG_TIDY ${CPPCHECK})

            # set(CMAKE_C_NCLUDE_WHAT_YOU_USE ${CPPCHECK})
            # set(CMAKE_CXX_NCLUDE_WHAT_YOU_USE ${CPPCHECK})

        endif()
    endfunction()
else()
    function(setup_cppcheck target)
    endfunction()
endif()
