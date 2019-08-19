#!/usr/bin/env bash

cmake -H. -Bbuild -DCMAKE_CXX_COMPILER=/usr/bin/g++-8 -DCMAKE_C_COMPILER=/usr/bin/gcc-8 -DCMAKE_BUILD_TYPE=$1 -DARK_BUILD_EXE=1 -DBUILD_MODULES=true
