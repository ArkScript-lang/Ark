#!/bin/bash

set -ev

cmake -H. -Bbuild -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=1
cmake --build build --config Release

if [ -f build/Release/Ark ]; then
    mv build/Release/Ark build/Ark
fi
build/Ark tests/unittest.ark --lib lib/