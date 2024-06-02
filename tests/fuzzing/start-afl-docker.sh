if [[ $(ls -l | grep tests) == "" ]]; then
  echo "This script needs to run at the root of ArkScript-lang/Ark"
  exit 1
fi

# we need a ton of ramdisks, one per fuzzer, to avoid killing the
# disk with millions of writes when cmin/tmin/fuzz are running
docker run -it --rm --name afldocker \
  --mount type=tmpfs,destination=/ramdisk \
  --mount type=tmpfs,destination=/ramdisk2 \
  --mount type=tmpfs,destination=/ramdisk3 \
  --mount type=tmpfs,destination=/ramdisk4 \
  -e AFL_TMPDIR=/ramdisk \
  -v $(pwd):/src \
  aflplusplus/aflplusplus:v4.20c
