# custom commands for UNIX/WINDOWS host systems
# 1) use cppcheck for static analysis of the code
# 2) copy parameter/profile data files into the build directory

if(CPPCHECK)
    # create the compile_commands.json file as cppcheck project file
    set(CPPCHECK_PRJ_IN compile_commands.json)
    set(CPPCHECK_PRJ_FILE cppcheck.json)
    set(MISRA_ADDON "misra.json")
    set(MISRA_SCRIPT $<IF:$<BOOL:${MINGW}>,"$ENV{MSYSTEM_PREFIX}/share/cppcheck/addons/misra.py","misra">)

    # copy compile_commands.json in order 
    add_custom_command(OUTPUT ${CPPCHECK_PRJ_FILE}
        VERBATIM
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${CPPCHECK_PRJ_IN}
        COMMAND ${CMAKE_COMMAND} -E copy ${CPPCHECK_PRJ_IN} ${CPPCHECK_PRJ_FILE}
        COMMENT "Copy ${CPPCHECK_PRJ_IN} for use with 'cppcheck' (=> ${CPPCHECK_PRJ_FILE})"
    )

    if(CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
        # patch compile_commands.json in order
        add_custom_command(OUTPUT ${CPPCHECK_PRJ_FILE}
            APPEND
            VERBATIM
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND sed -i -e "s;/\\([A-Z]\\)/;\\1:/;g" ${CPPCHECK_PRJ_FILE}
            COMMENT "Patch ${CPPCHECK_PRJ_IN} for use with 'cppcheck' (=> ${CPPCHECK_PRJ_FILE})"
        )
    endif()

    add_custom_command(OUTPUT ${MISRA_ADDON}
        VERBATIM    
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "{" > ${MISRA_ADDON}
        COMMAND ${CMAKE_COMMAND} -E echo "    \"script\": ${MISRA_SCRIPT}," >> ${MISRA_ADDON}
        COMMAND ${CMAKE_COMMAND} -E echo "    \"args\": [" >> ${MISRA_ADDON}
        COMMAND ${CMAKE_COMMAND} -E echo "    ]" >> ${MISRA_ADDON}
        COMMAND ${CMAKE_COMMAND} -E echo "}" >> ${MISRA_ADDON}
        COMMAND ${CMAKE_COMMAND} -E echo "" >> ${MISRA_ADDON}
        COMMENT "Create 'misra.json' for use with 'cppcheck'"
    )

    # call cppcheck to perform the static source code analysis
    add_custom_target(cppcheck ALL
        VERBATIM
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${CPPCHECK_PRJ_FILE} misra.json
        COMMAND cppcheck --template=gcc --project=${CPPCHECK_PRJ_FILE} $<$<BOOL:${CONFIG_CHECKMISRA}>:--addon=${MISRA_ADDON}> $<$<BOOL:${WIN32}>:-D__WIN32__>
        # COMMAND ${CMAKE_COMMAND} -E rm -f ${CPPCHECK_PRJ_FILE} ${MISRA_ADDON}
        COMMENT "Perform a static code analysis using 'cppcheck'"
    )
endif()

if(FALSE)
if(WIN32 OR UNIX)
    # copy the parameter/profile data files into the build directory
    file(GLOB PAR_PROF LIST_DIRECTORIES FALSE "USC2_P*.bin")
    file(GLOB SDF LIST_DIRECTORIES FALSE "BSP/COMMON/*.sdf")
    add_custom_target(ProfPar ALL
        VERBATIM
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS USC2_Parameter.bin USC2_Profile.bin
        COMMAND ${CMAKE_COMMAND} -E copy ${PAR_PROF} ${SDF} ${CMAKE_BINARY_DIR}
        COMMENT "Copy parameter and profile data files (${DAT_FILES})"
    )
endif()
endif()
