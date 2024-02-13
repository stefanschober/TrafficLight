# QM/Source
target_sources(${TGT} PRIVATE
    TLmain.c
    TLblinker.c
    TLtraffic.c
    TLpedestrian.c
)
target_include_directories(${TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
