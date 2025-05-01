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

export AFL_MAP_SIZE=223723

rm -rf tests/fuzzing/corpus-cmin/*
rm -rf tests/fuzzing/corpus-cmin-tmin/*
mkdir -p tests/fuzzing/corpus-cmin
mkdir -p tests/fuzzing/corpus-cmin-tmin
afl-cmin -i tests/fuzzing/corpus -o tests/fuzzing/corpus-cmin -T all -- "$exe" @@ -L "$ark_lib"

cd tests/fuzzing/corpus-cmin || exit 1

cores=$(nproc)
input_dir="."
output_dir="../corpus-cmin-tmin"
# shellcheck disable=SC2012
total=$(ls "$input_dir" | wc -l)

for k in $(seq 1 "${cores}" "${total}"); do
	for i in $(seq 0 $(("$cores" - 1))); do
		# shellcheck disable=SC2012
		file=$(ls -Sr $input_dir | sed $(("$i" + "$k"))"q;d")
		afl-tmin -i "$input_dir/$file" -o "$output_dir/$file" -- "$exe" @@ -L "$ark_lib" & #put the command to run after the --
	done

	wait
done
