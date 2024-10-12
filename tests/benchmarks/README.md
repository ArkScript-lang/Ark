# Benchmarks

## Running the benchmarks and storing the results

1. Run for the repository root
2. Make sure the target `arkscript` has been compiled in release mode, as well as `bench`
```bash
result="tests/benchmarks/results/$(set -- tests/benchmarks/results/*.csv; echo $#)-$(git rev-parse --short HEAD).csv"
cmake-build-release/bench \
  --benchmark_min_warmup_time=1 \
  --benchmark_format=csv \
  --benchmark_time_unit=ms \
  --v=0 | grep -Ev "(New parser|Welder)" > $result
```

## Generate the comparison

```bash
python3 tests/benchmarks/compare.py tests/benchmarks/results/*.csv
```
