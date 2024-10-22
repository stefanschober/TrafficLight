# setup GTK 3.0 support
if(CONFIG_RASPI)
    set(ENV{PKG_CONFIG_PATH} "/usr/lib/arm-linux-gnueabihf/pkgconfig/")
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 gtk+-3.0) # QUIET IMPORTED_TARGET)

if (NOT GTK3_FOUND)
    message(FATAL_ERROR "The package GTK3, required for this build, could not be found on the system!")
endif()

# Add other flags to the compiler
target_compile_options(${TGT} PRIVATE ${GTK3_CFLAGS_OTHER})

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
target_include_directories(${TGT} PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_directories(${TGT} PRIVATE ${GTK3_LIBRARY_DIRS})
target_link_options(${TGT} PRIVATE ${GTK3_LDFLAGS_OTHER})

# Link the target to the GTK+ libraries
target_link_libraries(${TGT} PRIVATE ${GTK3_LIBRARIES}) # PkgConfig::GTK3)
