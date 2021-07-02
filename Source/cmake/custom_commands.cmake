# custom commands for UNIX/WINDOWS host systems
# 1) use cppcheck for static analysis of the code
# 2) copy parameter/profile data files into the build directory

if(WINDOWS OR WIN32 OR UNIX)
    # check if the cppcheck tool is properly installed and can be found
    # via the PATH environment variable
    if(CONFIG_CPPCHECK)
        find_program(CPPCHECK NAMES cppcheck cppcheck.exe)
        if(CPPCHECK)
            # create the compile_commands.json file as cppcheck project file
            set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
            set(CPPCHECK_PRJ_IN compile_commands.json)
            set(CPPCHECK_PRJ_FILE cppcheck.json)

            if(WINDOWS OR WIN32)
                # patch compile_commands.json in order 
                add_custom_command(TARGET ${TGT} POST_BUILD VERBATIM
                    COMMAND ${CMAKE_COMMAND} -E copy ${CPPCHECK_PRJ_IN} ${CPPCHECK_PRJ_FILE}
                    COMMAND sed -i -e "s;/\\([A-Z]\\)/;\\1:/;g" ${CPPCHECK_PRJ_FILE}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Patch ${CPPCHECK_PRJ_IN} for use with 'cppcheck' (=> ${CPPCHECK_PRJ_FILE})"
                )
            else()
                # copy compile_commands.json in order 
                add_custom_command(TARGET ${TGT} POST_BUILD VERBATIM
                    COMMAND ${CMAKE_COMMAND} -E copy ${CPPCHECK_PRJ_IN} ${CPPCHECK_PRJ_FILE}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Copy ${CPPCHECK_PRJ_IN} for use with 'cppcheck' (=> ${CPPCHECK_PRJ_FILE})"
                )
            endif()

            # call cppcheck to perform the static source code analysis
            add_custom_command(TARGET ${TGT} POST_BUILD VERBATIM
                COMMAND cppcheck --template=gcc --project=${CPPCHECK_PRJ_FILE} $<$<BOOL:WIN32>:-D__WIN32__>
                COMMAND rm -vf "${CPPCHECK_PRJ_FILE}"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Perform a static code analysis using 'cppcheck'"
            )
        endif()
    endif()

    # copy the parameter/profile data files into the build directory
    add_custom_command(TARGET ${TGT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy USC2_Parameter.bin USC2_Profile.bin BSP/Common/*.sdf ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Copy parameter and profile data files (${DAT_FILES})"
    )
endif()
