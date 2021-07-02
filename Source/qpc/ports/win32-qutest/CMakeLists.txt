# ./ports/win32-qutest
set(_src qutest_port.c)
set(_inc ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${QPLIB} PRIVATE ${_src})
target_include_directories(${QPLIB} PUBLIC ${_inc})
listIncludes(${CMAKE_CURRENT_SOURCE_DIR} ${_inc})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

