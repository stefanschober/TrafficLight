#! /bin/bash
[ -n "$1" ] && GENERATOR="$1" || GENERATOR="Unix Makefiles"
CMAKE="../CMake/build/bin/cmake"
SRCDIR="/home/stefan/Projects/TrafficLight/Source"
BLDDIR="/home/stefan/Projects/TrafficLight/Build/Linux_Make"

${CMAKE} --fresh -G "${GENERATOR}" \
    -S "${SRCDIR}" \
    -B "${BLDDIR}" \
    -DCONFIG_KERNEL_QK=TRUE \
    -DCONFIG_KERNEL_QV=FALSE \
    -DCONFIG_GUI=ON \
    -DCONFIG_CPPCHECK=TRUE \
    -DCONFIG_CHECKMISRA=TRUE \
    -DCONFIG_QSPY=FALSE \
    -DCONFIG_VERBOSE=FALSE \
    -DCONFIG_DEBUG=FALSE

${CMAKE} --build "${BLDDIR}"