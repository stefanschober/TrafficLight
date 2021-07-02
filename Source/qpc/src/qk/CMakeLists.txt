# ./src/qk
set(_src qk.c)
target_sources(${QPLIB} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

