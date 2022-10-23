#!/usr/bin/env bash

if [[ $(basename $(pwd)) != "Ark" ]]; then
    echo "This script should be launched from ArkScript source folder" && exit 1
fi

buildFolder=afl

sudo afl-system-config
afl-fuzz -i fuzzing/input -o fuzzing/output -s 0 -- ${buildFolder}/arkscript @@ -L ./lib
