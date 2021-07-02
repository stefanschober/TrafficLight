# ./src/qv
set(_src qv.c)
target_sources(${QPLIB} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

