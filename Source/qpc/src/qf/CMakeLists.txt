# ./src/qf
set(_src "")
list(APPEND _src qep_hsm.c qep_msm.c qf_act.c qf_actq.c qf_defer.c qf_dyn.c qf_mem.c)
list(APPEND _src qf_ps.c qf_qact.c qf_qeq.c qf_qmact.c qf_time.c)
target_sources(${QPLIB} PRIVATE ${_src})
listModules(${CMAKE_CURRENT_SOURCE_DIR} ${_src})

