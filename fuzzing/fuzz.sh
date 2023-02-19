#!/usr/bin/env bash

if [[ $(basename $(pwd)) != "Ark" ]]; then
    echo "This script should be launched from ArkScript source folder" && exit 1
fi

instance="-M main"
if [[ $# == 1 ]]; then
    instance="-S variant-$1"
fi

buildFolder=afl

# load tests cases from other fuzzers first
export AFL_IMPORT_FIRST=1

afl-fuzz -i fuzzing/input \
        -o fuzzing/output \
        -s 0 \
        -m 64 \
        $instance \
        -- ${buildFolder}/arkscript @@ -L ./lib
