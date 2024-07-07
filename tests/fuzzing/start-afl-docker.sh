if [[ $(ls -l | grep tests) == "" ]]; then
  echo "This script needs to run at the root of ArkScript-lang/Ark"
  exit 1
fi
if [[ "$TERM" =~ "screen".* ]]; then
  echo "This script shouldn't be running inside tmux!"
  exit 1
fi


# we need a ton of ramdisks, one per fuzzer, to avoid killing the
# disk with millions of writes when cmin/tmin/fuzz are running
docker run -it --rm --name afldocker \
  --mount type=tmpfs,destination=/ramdisk0 \
  --mount type=tmpfs,destination=/ramdisk1 \
  --mount type=tmpfs,destination=/ramdisk2 \
  --mount type=tmpfs,destination=/ramdisk3 \
  --mount type=tmpfs,destination=/ramdisk4 \
  -v $(pwd):/src \
  aflplusplus/aflplusplus:v4.20c \
  bash /src/tests/fuzzing/docker/setup-container.sh
