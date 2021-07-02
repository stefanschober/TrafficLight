# ./src/qs
set(_src "")
list(APPEND _src qs.c qs_64bit.c qs_fp.c qs_rx.c)
if(CONFIG_UNIT_TEST)
    list(APPEND _src qutest.c)
endif()
target_sources(${QPLIB} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

