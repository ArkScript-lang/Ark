#!/usr/bin/bash

if [[ $(basename $(pwd)) != "Ark" ]]; then
    echo "This script should be launched from ArkScript source folder" && exit 1
fi

buildFolder=afl

cmake -B${buildFolder} \
    -DCMAKE_C_COMPILER=afl-cc -DCMAKE_CXX_COMPILER=afl-c++ -DCMAKE_BUILD_TYPE=Release \
    -DARK_BUILD_EXE=On -DARK_BUILD_MODULES=On
cmake --build ${buildFolder} --config Release
