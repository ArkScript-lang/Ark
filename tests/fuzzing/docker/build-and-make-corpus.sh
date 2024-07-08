if ! [ -f /.dockerenv ]; then
  echo "This script needs to run inside the aflplusplus docker container"
  exit 1
fi

cd /src || exit 1

###################################################
# Build ArkScript
###################################################
source ./tests/fuzzing/docker/0-build-with-afl.sh

###################################################
# Generate corpus
###################################################
source ./tests/fuzzing/docker/1-prepare-corpus.sh
