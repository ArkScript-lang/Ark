if ! [ -f /.dockerenv ]; then
  echo "This script needs to run inside the aflplusplus docker container"
  exit 1
fi

cd /src || exit 1

###################################################
# Install utilities
###################################################

apt update -yq
apt install -yq tmux

###################################################
# Build ArkScript
###################################################
source ./tests/fuzzing/docker/1-build-with-afl.sh

###################################################
# Launch the fuzzers
###################################################
echo "Starting main fuzzer" && tmux new-session -d './tests/fuzzing/docker/2-fuzz.sh 0'
sleep 20  # wait for the main fuzzer to be up and ready

echo "Starting sub-fuzzer" && tmux new-session -d './tests/fuzzing/docker/2-fuzz.sh 1'
echo "Starting sub-fuzzer" && tmux new-session -d './tests/fuzzing/docker/2-fuzz.sh 2'
echo "Starting sub-fuzzer" && tmux new-session -d './tests/fuzzing/docker/2-fuzz.sh 3'
echo "Starting sub-fuzzer" && tmux new-session -d './tests/fuzzing/docker/2-fuzz.sh 4'

tmux attach 0
