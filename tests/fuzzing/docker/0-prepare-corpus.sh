if ! [ -f /.dockerenv ]; then
  echo "This script needs to run inside the aflplusplus docker container"
  exit 1
fi
if [[ $(ls -l | grep tests) == "" ]]; then
  echo "This script needs to run at the root of ArkScript-lang/Ark"
  exit 1
fi

exe=$(pwd)/build/arkscript
ark_lib=$(pwd)/lib

rm -rf tests/fuzzing/corpus-cmin/*
rm -rf tests/fuzzing/corpus-cmin-tmin/*
afl-cmin -i tests/fuzzing/corpus -o tests/fuzzing/corpus-cmin -T all -- "$exe" @@ -L "$ark_lib"

cd tests/fuzzing/corpus-cmin || exit 1
for i in *.ark; do
  afl-tmin -i "$i" -o "../corpus-cmin-tmin/$i" -- "$exe" @@ -L "$ark_lib"
done
