if ! [ -f /.dockerenv ]; then
  echo "This script needs to run inside the aflplusplus docker container"
  exit 1
fi
if [[ $(ls -l | grep tests) == "" ]]; then
  echo "This script needs to run at the root of ArkScript-lang/Ark"
  exit 1
fi

cd /src || exit 1

cmake -Bbuild \
  -DCMAKE_C_COMPILER=/AFLplusplus/afl-cc \
  -DCMAKE_CXX_COMPILER=/AFLplusplus/afl-c++ \
  -DCMAKE_BUILD_TYPE=Release \
  -DARK_BUILD_EXE=On \
  -DARK_SANITIZERS=On
cmake --build build --config Release -j $(nproc)
