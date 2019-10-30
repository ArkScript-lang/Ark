#!/bin/bash

set -ev

wget https://www.sfml-dev.org/files/SFML-2.5.1-sources.zip
unzip SFML-2.5.1-sources.zip -d SFML
cd SFML
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CXX_COMPILER}
cmake --build build --config Release
cmake --install build --config Release
cd ..

cmake -S. -Bbuild -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=1 -DARK_BUILD_MODULES=1
cmake --build build --config Release

if [ -f build/Release/Ark ]; then
    mv build/Release/Ark build/Ark
fi
build/Ark tests/unittests.ark --lib lib/ || exit 1