#! /bin/bash

configurations="debug release spy"

for p in {mingw,arm}-{qv,qk}
do
    echo "Configuring for preset '$p'"
    cmake --preset=$p --fresh .
    for c in ${configurations}
    do
        echo "Building for configuration '$c'"
        cmake --build --preset=$p --config=$c --clean-first
    done
done
