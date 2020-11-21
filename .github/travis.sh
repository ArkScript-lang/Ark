#!/bin/bash

set -ev

cmake . -Bbuild -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=On -DARK_BUILD_MODULES=On
cmake --build build --config Release --target ark
cmake --build build --config Release --target console

if [ -f build/Release/ark ]; then
    mv build/Release/ark build/ark
fi
build/ark tests/unittests.ark --lib lib/ || exit 1