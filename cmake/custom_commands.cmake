# custom commands for UNIX/WINDOWS host systems
# 1) use cppcheck for static analysis of the code
# 2) copy parameter/profile data files into the build directory

# copy the parameter/profile data files into the build directory
if(FALSE)
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
