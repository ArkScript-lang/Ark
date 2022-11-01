#!/usr/bin/env bash

if [[ $(basename $(pwd)) != "Ark" ]]; then
    echo "This script should be launched from ArkScript source folder" && exit 1
fi

buildFolder=afl

afl-cmin -i fuzzing/corpus -o fuzzing/unique -- ${buildFolder}/arkscript @@ -L ./lib

mkdir -p fuzzing/input
cd fuzzing/unique

for i in *; do
  afl-tmin -i "$i" -o "../input/$i" -- ../../${buildFolder}/arkscript @@ -L ../../lib
done

cd ../..
