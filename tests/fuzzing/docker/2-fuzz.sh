if ! [ -f /.dockerenv ]; then
  echo "This script needs to run inside the aflplusplus docker container"
  exit 1
fi
if [[ $(ls -l | grep tests) == "" ]]; then
  echo "This script needs to run at the root of ArkScript-lang/Ark"
  exit 1
fi

if [[ $# != 1 ]]; then
  echo "Usage: fuzz.sh [campaign id]"
  echo "  0: main"
  echo "  1: old queue cycling"
  echo "  2: mOPT"
  echo "  3: exploit"
  echo "  4: explore"

  exit 1
fi

export AFL_IMPORT_FIRST=1
export AFL_USE_ASAN=1
export AFL_USE_UBSAN=1
export AFL_TMPDIR=/ramdisk$1
export AFL_AUTORESUME=1

export FUZZER_SEED=0
export FUZZER_TIMEOUT_EXEC_MS=500
export BUILD_FOLDER=build

# 50-70% of the time
# export AFL_DISABLE_TRIM=1

DEFAULT_ARGS="-i tests/fuzzing/corpus-cmin-tmin -o output -a ascii -s $FUZZER_SEED -t $FUZZER_TIMEOUT_EXEC_MS -- ${BUILD_FOLDER}/arkscript @@ -L /src/lib"

case $1 in
  0)
    afl-fuzz -M main $DEFAULT_ARGS
    ;;
  1)
    afl-fuzz -S oldqueuecycling -Z $DEFAULT_ARGS
    ;;
  2)
    afl-fuzz -S mopt -L 0 $DEFAULT_ARGS
    ;;
  3)
    afl-fuzz -S exploit -P exploit $DEFAULT_ARGS
    ;;
  4)
    afl-fuzz -S explore -P explore $DEFAULT_ARGS
    ;;
  *)
    echo "Unknown campaign $1"
esac
