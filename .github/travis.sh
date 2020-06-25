#!/bin/bash

set -ev

cmake . -Bbuild -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=1 -DARK_BUILD_BASE_MODULES=1
cmake --build build --config Release --target Ark
cmake --build build --config Release --target console

if [ -f build/Release/Ark ]; then
    mv build/Release/Ark build/Ark
fi
build/Ark tests/unittests.ark --lib lib/ || exit 1